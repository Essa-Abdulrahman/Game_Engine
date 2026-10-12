#pragma once

// Public umbrella header for applications built on Odyssey (editor, runtime, tests).
// Engine code must include the specific headers it needs instead.

#include "Odyssey/Core/Base.h"
#include "Odyssey/Core/Assert.h"
#include "Odyssey/Core/Buffer.h"
#include "Odyssey/Core/CommandLine.h"
#include "Odyssey/Core/Environment.h"
#include "Odyssey/Core/FileSystem.h"
#include "Odyssey/Core/Hash.h"
#include "Odyssey/Core/KeyCodes.h"
#include "Odyssey/Core/Layer.h"
#include "Odyssey/Core/LayerStack.h"
#include "Odyssey/Core/Log.h"
#include "Odyssey/Core/StringUtils.h"
#include "Odyssey/Core/Timer.h"
#include "Odyssey/Core/Timestep.h"
#include "Odyssey/Core/UUID.h"

#include "Odyssey/Events/ApplicationEvent.h"
#include "Odyssey/Events/Event.h"
#include "Odyssey/Events/KeyEvent.h"
#include "Odyssey/Events/MouseEvent.h"
