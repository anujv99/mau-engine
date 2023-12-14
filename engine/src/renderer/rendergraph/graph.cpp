#include "graph.h"

#include "graphics/vulkan-state.h"

namespace mau {

  RenderGraph::RenderGraph() { }

  RenderGraph::~RenderGraph() { }

  void RenderGraph::AddPass(Handle<Pass> pass) {
    m_Passes.push_back(pass);
  }

  void RenderGraph::Build(const std::vector<Sink>& global_sinks) {
    m_GlobalSinks.clear();

    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();

    // setup global resources
    std::vector<Handle<Resource>> swapchain_images = {};
    std::vector<Handle<Resource>> swapchain_depth_images = {};
    for (size_t i = 0; i < swapchain->GetImages().size(); i++) {
      Handle<ImageResource> backbuffer = make_handle<ImageResource>(swapchain->GetImages()[i], swapchain->GetImageViews()[i]);
      swapchain_images.push_back(backbuffer);

      Handle<ImageResource> depthbuffer = make_handle<ImageResource>(swapchain->GetDepthImages()[i], swapchain->GetDepthImageViews()[i]);
      swapchain_depth_images.push_back(depthbuffer);
    }

    Sink backbuffer("$backbuffer");
    backbuffer.AssignResources(swapchain_images);

    Sink depthbuffer("$depthbuffer");
    depthbuffer.AssignResources(swapchain_depth_images);

    m_GlobalSinks.insert(std::make_pair(backbuffer.GetName(), backbuffer));
    m_GlobalSinks.insert(std::make_pair(depthbuffer.GetName(), depthbuffer));

    for (const auto& sink : global_sinks) {
      m_GlobalSinks.insert(std::make_pair(sink.GetName(), sink));
    }

    // setup passes
    for (auto& pass : m_Passes) {
      pass->Build(m_GlobalSinks, static_cast<TUint32>(swapchain_images.size()));

      const UnorderedMap<String, Sink>& pass_sinks = pass->GetSinks();
      
      for (auto& sink : pass_sinks) {
        m_GlobalSinks.insert(std::make_pair(sink.first, sink.second));
      }
    }

    // finished
  }

  void RenderGraph::Execute(Handle<CommandBuffer> cmd, TUint32 current_Frame) {
    for (auto& pass : m_Passes) {
      pass->Execute(cmd, current_Frame);
    }
  }

}
