#include "../Include/RenderPipelineM.h"

int RPMTreap::GetRank(RPMNode *x, RPMNode *q) {
    if(!x)return 0;
    if(q->operator<(x))return GetRank(x->left, q);
    else return GetRank(x->right, q) + 1 + x->lsize();
}

RPMNode *RPMTreap::DoInsert(RPMNode *x, RPMNode *y) {
    if(!x)return y;
    if(!y)return x;
    if (x->weight() < y->weight()) {
        x->right = DoInsert(x->right, y);
        x->Pushup();
        return x;
    }
    else {
        y->left = DoInsert(x, y->left);
        y->Pushup();
        return y;
    }
}

RPMTreap::droot RPMTreap::DoSplit(RPMNode *x, int k) {
    auto r = droot(nullptr, nullptr);
    if(!x)return r;
    if (x->lsize() >= k) {
        r = DoSplit(x->left, k);
        x->left = r.second;
        x->Pushup();
        r.second = x;
    }
    else {
        r = DoSplit(x->right, k - x->lsize() - 1);
        x->right = r.first;
        x->Pushup();
        r.first = x;
    }
    return r;
}

void RPMTreap::DoRender(RPMNode *u) {
    if (!u)return;
    if (u->right)DoRender(u->right);
    ID3D11CommandList *l = u->rpm->ReadRef().load();
    if (l) {
        l->AddRef();
        device->native_immcontext->ExecuteCommandList(l, false);
        l->Release();
    }
    if(u->left)DoRender(u->left);
}

void RPMTreap::DoClear(RPMNode *u) {
    if(!u)return;
    DoClear(u->left);
    DoClear(u->right);
    delete u;
}

void RPMTreap::DoErase(RenderPipelineM *rpm) {
    RPMNode *d = uid2node[rpm->access.second];
    if(!d)return;
    uid2node.erase(rpm->access.second);
    rpm->access = {0, 0};
    int k = GetRank(root, d);
    auto p = DoSplit(root, k);
    auto p2 = DoSplit(p.first, k-1);
    if(p2.second)delete p2.second;
    root = DoInsert(p2.first, p.second);
}

namespace Ext {
    namespace DX {
        namespace RenderPipeline {
            VALUE klass_rpm;
            VALUE klass_remote_render_executive;

            static VALUE rpm_swap_commands(VALUE self) {
                GetNativeObject<::RenderPipelineM>(self)->SwapCommands();
                return self;
            }

            //------------------------
            static VALUE RE_initialize(VALUE self, VALUE device, VALUE swp, VALUE fps) {
                auto re = GetNativeObject<RemoteRenderExecutive>(self);
                re->Initialize(GetNativeObject<::D3DDevice>(device),
                    GetNativeObject<::SwapChain>(swp), FIX2INT(fps));
                return self;
            }
            static VALUE RE_terminate(VALUE self) {
                GetNativeObject<RemoteRenderExecutive>(self)->Terminate();
                return self;
            }
            static VALUE RE_reset_fps(VALUE self, VALUE fps) {
                GetNativeObject<RemoteRenderExecutive>(self)->ResetFPS(FIX2INT(fps));
                return self;
            }
            static VALUE RE_insert(VALUE self, VALUE rpm, VALUE priority) {
                //       if(!rb_obj_is_kind_of(rp, DX::RenderPipeline::klass))
                //           rb_raise(rb_eArgError, "RemoteRenderExecutive#push : param should be a DX::RenderPipeline");
                CheckArgs({rpm, priority}, {klass_rpm, rb_cInteger});
                GetNativeObject<RemoteRenderExecutive>(self)->Insert(GetNativeObject<::RenderPipelineM>(rpm), FIX2INT(priority));
                return self;
            }
            static VALUE RE_clear(VALUE self) {
                GetNativeObject<RemoteRenderExecutive>(self)->Clear();
                return self;
            }
            static VALUE RE_lock(VALUE self) {
                GetNativeObject<RemoteRenderExecutive>(self)->Lock();
                return self;
            }
            static VALUE RE_unlock(VALUE self) {
                GetNativeObject<RemoteRenderExecutive>(self)->UnLock();
                return self;
            }
            static VALUE RE_erase(VALUE self, VALUE rpm) {
                CheckArgs({ rpm }, {klass_rpm});
                GetNativeObject<RemoteRenderExecutive>(self)->Erase(GetNativeObject<RenderPipelineM>(rpm));
                return self;
            }

            void InitRPM() {
                klass_rpm = rb_define_class_under(module, "RenderPipelineM", RenderPipeline::klass);
                rb_define_alloc_func(klass_rpm, RefObjNew<::RenderPipelineM>);
                rb_define_method(klass_rpm, "swap_commands", (rubyfunc)rpm_swap_commands, 0);

                //RE
                klass_remote_render_executive = rb_define_class_under(module, "RemoteRenderExecutive", rb_cObject);
                rb_include_module(klass_remote_render_executive, module_release);
                rb_define_alloc_func(klass_remote_render_executive, RefObjNew<::RemoteRenderExecutive>);
                rb_define_method(klass_remote_render_executive, "initialize", (rubyfunc)RE_initialize, 3);
                rb_define_method(klass_remote_render_executive, "reset_fps", (rubyfunc)RE_reset_fps, 1);
                rb_define_method(klass_remote_render_executive, "insert", (rubyfunc)RE_insert, 2);
                rb_define_method(klass_remote_render_executive, "clear", (rubyfunc)RE_clear, 0);
                rb_define_method(klass_remote_render_executive, "terminate", (rubyfunc)RE_terminate, 0);
                rb_define_method(klass_remote_render_executive,"lock", (rubyfunc)RE_lock, 0);
                rb_define_method(klass_remote_render_executive, "unlock", (rubyfunc)RE_unlock, 0);
                rb_define_method(klass_remote_render_executive, "erase", (rubyfunc)RE_erase, 1);
            }
        }
    }
}