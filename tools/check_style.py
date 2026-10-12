#!/usr/bin/env python3
"""Odyssey style checker.

Enforces the mechanical rules from AGENTS.md that clang-format and clang-tidy do not cover, verifies that every
source file is listed in its CMakeLists.txt, and (optionally) runs clang-format in check or fix mode.

Usage:
    python3 tools/check_style.py                         # lint rules + clang-format if it is installed
    python3 tools/check_style.py --fix                   # apply clang-format, then lint
    python3 tools/check_style.py --clang-format clang-format-18 --require-clang-format   # CI
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

REPOSITORY_ROOT = Path(__file__).resolve().parent.parent

# First-party targets: (directory containing CMakeLists.txt, source root). Missing directories are skipped,
# so targets added in later phases are picked up automatically once listed here.
TARGETS = [
    ("Odyssey", "Odyssey/src"),
    ("OdysseyTests", "OdysseyTests/src"),
    ("OdysseyEditor", "OdysseyEditor/src"),
    ("OdysseyRuntime", "OdysseyRuntime/src"),
]

SOURCE_SUFFIXES = {".h", ".cpp"}
ENGINE_SOURCE_ROOT = "Odyssey/src"
PCH_NAME = "odpch.h"

# Files allowed to write to the console directly.
CONSOLE_OUTPUT_ALLOWLIST = {"Odyssey/src/Odyssey/Core/Log.cpp"}

FILE_NAME_PATTERN = re.compile(r"^[A-Z][A-Za-z0-9]*\.(h|cpp)$")
FILE_NAME_EXCEPTIONS = {PCH_NAME}

PLATFORM_HEADER_PATTERN = re.compile(r'#\s*include\s*[<"](windows\.h|Windows\.h|X11/|Cocoa/|AppKit/|objc/)')
CONSOLE_OUTPUT_PATTERN = re.compile(r"\bstd::(cout|cerr|clog)\b|(?<![\w:])printf\s*\(|\bputs\s*\(")
NULL_PATTERN = re.compile(r"\bNULL\b")
USING_NAMESPACE_PATTERN = re.compile(r"^\s*using\s+namespace\b")
INCLUDE_PATTERN = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')


@dataclass
class Violation:
    path: Path
    line: int
    message: str

    def __str__(self) -> str:
        relative = self.path.relative_to(REPOSITORY_ROOT).as_posix()
        location = f"{relative}:{self.line}" if self.line > 0 else relative
        return f"{location}: {self.message}"


def collect_sources() -> list[Path]:
    sources: list[Path] = []
    for _, source_root in TARGETS:
        root = REPOSITORY_ROOT / source_root
        if root.is_dir():
            sources.extend(sorted(p for p in root.rglob("*") if p.suffix in SOURCE_SUFFIXES and p.is_file()))
    return sources


def strip_comments_and_strings(line: str, in_block_comment: bool) -> tuple[str, bool]:
    """Removes comments and string/char literals from a line so pattern checks only see code."""
    result = []
    i = 0
    while i < len(line):
        if in_block_comment:
            end = line.find("*/", i)
            if end == -1:
                return "".join(result), True
            i = end + 2
            in_block_comment = False
            continue
        if line.startswith("//", i):
            break
        if line.startswith("/*", i):
            in_block_comment = True
            i += 2
            continue
        if line[i] in "\"'":
            quote = line[i]
            i += 1
            while i < len(line) and line[i] != quote:
                i += 2 if line[i] == "\\" else 1
            i += 1
            result.append(quote * 2)
            continue
        result.append(line[i])
        i += 1
    return "".join(result), in_block_comment


def check_file(path: Path) -> list[Violation]:
    violations: list[Violation] = []
    relative = path.relative_to(REPOSITORY_ROOT).as_posix()
    raw = path.read_bytes()

    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        return [Violation(path, 0, "file is not valid UTF-8")]

    if raw.startswith(b"\xef\xbb\xbf"):
        violations.append(Violation(path, 1, "file must not start with a UTF-8 byte order mark"))
    if b"\r" in raw:
        violations.append(Violation(path, 0, "file must use LF line endings"))
    if raw and not raw.endswith(b"\n"):
        violations.append(Violation(path, 0, "file must end with a newline"))

    if path.name not in FILE_NAME_EXCEPTIONS and not FILE_NAME_PATTERN.match(path.name):
        violations.append(Violation(path, 0, "file names must be PascalCase (eg. LayerStack.cpp)"))

    lines = text.split("\n")
    is_header = path.suffix == ".h"
    is_engine_source = relative.startswith(ENGINE_SOURCE_ROOT + "/")

    if is_header:
        first_code_line = next((l.strip() for l in lines if l.strip() and not l.strip().startswith("//")), "")
        if first_code_line != "#pragma once":
            violations.append(Violation(path, 1, "headers must start with '#pragma once'"))

    if is_engine_source and path.suffix == ".cpp":
        first_include = next((m.group(1) for m in map(INCLUDE_PATTERN.match, lines) if m), None)
        if first_include != PCH_NAME:
            violations.append(Violation(path, 1, f"engine source files must include \"{PCH_NAME}\" first"))

    in_block_comment = False
    for number, line in enumerate(lines, start=1):
        if line != line.rstrip():
            violations.append(Violation(path, number, "trailing whitespace"))
        if line.startswith(" ") and not in_block_comment and not line.lstrip().startswith("*"):
            violations.append(Violation(path, number, "indent with tabs, not spaces"))

        code, in_block_comment = strip_comments_and_strings(line, in_block_comment)

        if NULL_PATTERN.search(code):
            violations.append(Violation(path, number, "use nullptr instead of NULL"))
        if is_header and USING_NAMESPACE_PATTERN.match(code):
            violations.append(Violation(path, number, "'using namespace' is not allowed in headers"))
        if is_header and PLATFORM_HEADER_PATTERN.search(line):
            violations.append(Violation(path, number, "platform headers may only be included in platform .cpp files"))
        if is_engine_source and relative not in CONSOLE_OUTPUT_ALLOWLIST and CONSOLE_OUTPUT_PATTERN.search(code):
            violations.append(Violation(path, number, "engine code must log through OD_CORE_* instead of writing to the console"))

    return violations


def check_cmake_lists() -> list[Violation]:
    """Every source file must be listed in its target's CMakeLists.txt (sources are listed explicitly)."""
    violations: list[Violation] = []
    for target_directory, source_root in TARGETS:
        root = REPOSITORY_ROOT / source_root
        cmake_file = REPOSITORY_ROOT / target_directory / "CMakeLists.txt"
        if not root.is_dir():
            continue
        if not cmake_file.is_file():
            violations.append(Violation(root, 0, f"missing {target_directory}/CMakeLists.txt"))
            continue

        listed = set(re.findall(r"(src/[^\s\)\"]+\.(?:h|cpp))", cmake_file.read_text(encoding="utf-8")))
        for source in sorted(p for p in root.rglob("*") if p.suffix in SOURCE_SUFFIXES):
            entry = source.relative_to(REPOSITORY_ROOT / target_directory).as_posix()
            if entry not in listed:
                violations.append(Violation(source, 0, f"not listed in {target_directory}/CMakeLists.txt"))
        for entry in sorted(listed):
            if not (REPOSITORY_ROOT / target_directory / entry).is_file():
                violations.append(Violation(cmake_file, 0, f"lists missing file '{entry}'"))
    return violations


def run_clang_format(executable: str, sources: list[Path], fix: bool) -> bool:
    arguments = [executable, "-i"] if fix else [executable, "--dry-run", "--Werror"]
    chunk_size = 50
    success = True
    for start in range(0, len(sources), chunk_size):
        chunk = [str(p) for p in sources[start:start + chunk_size]]
        completed = subprocess.run(arguments + chunk, cwd=REPOSITORY_ROOT)
        success = success and completed.returncode == 0
    return success


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--fix", action="store_true", help="apply clang-format in place before linting")
    parser.add_argument("--clang-format", default="clang-format", help="clang-format executable to use")
    parser.add_argument("--require-clang-format", action="store_true", help="fail if clang-format is not available")
    arguments = parser.parse_args()

    sources = collect_sources()
    exit_code = 0

    clang_format = shutil.which(arguments.clang_format)
    if clang_format:
        if not run_clang_format(clang_format, sources, arguments.fix):
            print("clang-format: formatting differs (run: python3 tools/check_style.py --fix)")
            exit_code = 1
    elif arguments.require_clang_format or arguments.fix:
        print(f"error: '{arguments.clang_format}' not found")
        return 1
    else:
        print(f"note: '{arguments.clang_format}' not found, skipping formatting check")

    violations = check_cmake_lists()
    for source in sources:
        violations.extend(check_file(source))

    for violation in violations:
        print(violation)

    if violations:
        print(f"{len(violations)} style violation(s) in {len(sources)} file(s)")
        exit_code = 1
    else:
        print(f"style: {len(sources)} file(s) OK")
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
