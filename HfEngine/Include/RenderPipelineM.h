#pragma once
#include "RenderPipeline.h"
#include "Utility\swapdata.h"

#include <random>
#include <thread>
#include <mutex>
#include <unordered_map>
#include "D3DDevice.h"

class RenderPipelineM : public RenderPipeline, public SwapData<ComPtr<ID3D11CommandList>> {
    friend struct RPMNode;
    friend class RPMTreap;
    std::pair<int , unsigned> access; //pair<priority, uid>
public:
    void SwapCommands() {
        ID3D11CommandList *list;
        native_context->FinishCommandList(true, &list);
        WriteRef() = list;
        Swap();
    }
};

struct RPMNode {
    RenderPipelineM *rpm;
    int size = 1;
    RPMNode *left = nullptr, *right = nullptr;
    inline int lsize() {
        return left ? left->size : 0;
    }
    inline int rsize() {
        return right ? right->size : 0;
    }
    inline void Pushup() {
        size = lsize() + rsize() + 1;
    }
    inline bool operator<(RPMNode *o) {
        return rpm->access < o->rpm->access;
    }
    inline unsigned weight() {
        return rpm->access.second;
    }
};

class RPMTreap : Utility::ReferredObject {
    unsigned GenerateUid() {
        static std::default_random_engine engine;
        static std::uniform_int_distribution<unsigned> distribution(0, 65535);
        unsigned x = GetTickCount();
        return (x >> 16) + (distribution(engine) << 16);
    }
    typedef std::pair<RPMNode *, RPMNode *> droot;
        
    int GetRank(RPMNode *x, RPMNode *q);
    RPMNode *DoInsert(RPMNode *x, RPMNode *y);
    droot DoSplit(RPMNode *x, int k);
    void DoRender(RPMNode *u);

    std::unordered_map<unsigned, RPMNode *> uid2node;
    Utility::ReferPtr<D3DDevice> device;
    std::mutex tree_lock;
    RPMNode *root = nullptr;
public:
    inline void Insert(RenderPipelineM *rp, int priority) {
        rp->access.first = priority;
        rp->access.second = GenerateUid();
        RPMNode *node = new RPMNode;
        node->rpm = rp;

        std::lock_guard<std::mutex> g(tree_lock);
        uid2node[rp->access.second] = node;
        int k = GetRank(root, node);
        auto p = DoSplit(root, k);
        root = DoInsert(DoInsert(p.first, node), p.second);
    }

    void Initialize(D3DDevice *d) {
        device = d;
    }

    void UnInitialize() {
        device.Release();
        uid2node.clear();
    }

    virtual void Release() {
        UnInitialize();
    }

    void Render() {
        std::lock_guard<std::mutex> g(tree_lock);
        DoRender(root);
    }
};


class RemoteRenderExecutive : public Utility::ReferredObject {
    std::thread render_thread;
    std::mutex queue_lock;
    //moodycamel::ConcurrentQueue<ID3D11CommandList *> list_queue;
    std::queue<ID3D11CommandList *> list_queue;
    Utility::SleepFPSTimer timer;
    bool exit_flag;
public:
    Utility::ReferPtr<D3DDevice> device;
    Utility::ReferPtr<SwapChain> swapchain;
    int fps;
    void Initialize(D3DDevice *device_, SwapChain *swp, int fps_) {
        device = device_;
        swapchain = swp;
        fps = fps_;
        exit_flag = false;
        Run();
    }
    void Run() {
        render_thread = std::thread([this]() {
            ResetFPS(fps);
            while (!exit_flag) {
                ID3D11CommandList *clist = nullptr;
                if (!list_queue.empty()) {
                    queue_lock.lock();
                    while (!list_queue.empty()) {
                        clist = list_queue.front();
                        list_queue.pop();
                        device->native_immcontext->ExecuteCommandList(clist, false);
                        clist->Release();
                    }
                    queue_lock.unlock();
                }
                swapchain->Present();
                timer.Await();
            }
        });
    }
    inline void Push(RenderPipeline *rp) {
        ID3D11CommandList *list;
        rp->native_context->FinishCommandList(true, &list);
        if (!exit_flag) {
            queue_lock.lock();
            list_queue.push(list);
            queue_lock.unlock();
        }
    }
    inline void Terminate() {
        exit_flag = true;
        if (render_thread.joinable())
            render_thread.join();
        auto x = list_queue._Get_container();
        for (auto &l : x) {
            l->Release();
        }
        x.clear();
        UnInitialize();
    }
    inline void ResetFPS(int fps_) {
        timer.Restart(fps = fps_);
    }
    void UnInitialize() {
        if (render_thread.joinable()) {
            Terminate();
        }
        device.Release();
        swapchain.Release();
        //list_queue.~ConcurrentQueue();
    }
    virtual void Release() {
        UnInitialize();
    }
};


namespace Ext {
    namespace DX {
        namespace RenderPipeline {
            extern VALUE klass_rpm;
            extern VALUE klass_remote_render_executive;
            void InitRPM();
        }
    }
}