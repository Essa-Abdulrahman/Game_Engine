#include <doctest/doctest.h>

#include "TestUtilities.h"

#include "Odyssey/Core/LayerStack.h"

#include <string>
#include <vector>

using namespace Odyssey;

namespace {

	class RecordingLayer : public Layer
	{
	public:
		RecordingLayer(std::string name, std::vector<std::string>& journal)
			: Layer(std::move(name)), m_Journal(journal)
		{
		}

		~RecordingLayer() override { m_Journal.push_back("destroy " + GetName()); }

		void OnAttach() override { m_Journal.push_back("attach " + GetName()); }
		void OnDetach() override { m_Journal.push_back("detach " + GetName()); }
	private:
		std::vector<std::string>& m_Journal;
	};

	std::vector<std::string> GetOrder(const LayerStack& stack)
	{
		std::vector<std::string> names;
		for (const Scope<Layer>& layer : stack)
			names.push_back(layer->GetName());
		return names;
	}

}

TEST_CASE("LayerStack: layers stay below overlays regardless of push order")
{
	std::vector<std::string> journal;
	LayerStack stack;
	stack.PushOverlay(CreateScope<RecordingLayer>("Overlay1", journal));
	stack.PushLayer(CreateScope<RecordingLayer>("Layer1", journal));
	stack.PushOverlay(CreateScope<RecordingLayer>("Overlay2", journal));
	stack.PushLayer(CreateScope<RecordingLayer>("Layer2", journal));

	CHECK(GetOrder(stack) == std::vector<std::string> { "Layer1", "Layer2", "Overlay1", "Overlay2" });
	CHECK(stack.GetSize() == 4);
	CHECK(stack.GetLayerCount() == 2);
	CHECK(stack.GetOverlayCount() == 2);

	std::vector<std::string> reversed;
	for (auto it = stack.rbegin(); it != stack.rend(); ++it)
		reversed.push_back((*it)->GetName());
	CHECK(reversed == std::vector<std::string> { "Overlay2", "Overlay1", "Layer2", "Layer1" });
}

TEST_CASE("LayerStack: push attaches and returns the layer")
{
	std::vector<std::string> journal;
	LayerStack stack;
	Layer* layer = stack.PushLayer(CreateScope<RecordingLayer>("Game", journal));

	REQUIRE(layer != nullptr);
	CHECK(layer->GetName() == "Game");
	CHECK(journal == std::vector<std::string> { "attach Game" });
}

TEST_CASE("LayerStack: pop detaches and returns ownership")
{
	std::vector<std::string> journal;
	LayerStack stack;
	Layer* game = stack.PushLayer(CreateScope<RecordingLayer>("Game", journal));
	Layer* hud = stack.PushOverlay(CreateScope<RecordingLayer>("HUD", journal));
	journal.clear();

	Scope<Layer> poppedGame = stack.PopLayer(game);
	REQUIRE(poppedGame != nullptr);
	CHECK(poppedGame.get() == game);
	CHECK(journal == std::vector<std::string> { "detach Game" });
	CHECK(stack.GetLayerCount() == 0);
	CHECK(stack.GetOverlayCount() == 1);

	Scope<Layer> poppedHud = stack.PopOverlay(hud);
	CHECK(poppedHud.get() == hud);
	CHECK(stack.IsEmpty());

	poppedGame.reset();
	CHECK(journal.back() == "destroy Game");
}

TEST_CASE("LayerStack: popping unknown layers or from the wrong section returns null")
{
	std::vector<std::string> journal;
	LayerStack stack;
	Layer* layer = stack.PushLayer(CreateScope<RecordingLayer>("Layer", journal));
	Layer* overlay = stack.PushOverlay(CreateScope<RecordingLayer>("Overlay", journal));

	RecordingLayer stranger("Stranger", journal);
	CHECK(stack.PopLayer(&stranger) == nullptr);
	CHECK(stack.PopLayer(nullptr) == nullptr);
	CHECK(stack.PopLayer(overlay) == nullptr);
	CHECK(stack.PopOverlay(layer) == nullptr);
	CHECK(stack.GetSize() == 2);
}

TEST_CASE("LayerStack: destruction detaches and destroys top-most layers first")
{
	std::vector<std::string> journal;
	{
		LayerStack stack;
		stack.PushLayer(CreateScope<RecordingLayer>("A", journal));
		stack.PushLayer(CreateScope<RecordingLayer>("B", journal));
		stack.PushOverlay(CreateScope<RecordingLayer>("C", journal));
		journal.clear();
	}

	CHECK(journal == std::vector<std::string> { "detach C", "destroy C", "detach B", "destroy B", "detach A", "destroy A" });
}

TEST_CASE("LayerStack: Clear empties the stack and allows reuse")
{
	std::vector<std::string> journal;
	LayerStack stack;
	stack.PushLayer(CreateScope<RecordingLayer>("A", journal));
	stack.PushOverlay(CreateScope<RecordingLayer>("B", journal));
	stack.Clear();

	CHECK(stack.IsEmpty());
	CHECK(stack.GetLayerCount() == 0);

	stack.PushLayer(CreateScope<RecordingLayer>("C", journal));
	CHECK(GetOrder(stack) == std::vector<std::string> { "C" });
}

TEST_CASE("LayerStack: pushing null is verified")
{
	LayerStack stack;
	CHECK(Testing::CaptureAssertionFailure([&]() { stack.PushLayer(nullptr); }).has_value());
	CHECK(Testing::CaptureAssertionFailure([&]() { stack.PushOverlay(nullptr); }).has_value());
	CHECK(stack.IsEmpty());
}
