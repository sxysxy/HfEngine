#include "../Include/RenderPipelineM.h"

int GetRank(RPMNode *x, RPMNode *q) {
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
    if(!u)return;
    if(u->right)DoRender(u->right);
    auto &l = u->rpm->ReadRef();
    device->native_immcontext->ExecuteCommandList(l.Get(), false);
    if(u->left)DoRender(u->left);
}


namespace Ext {
    namespace DX {
        namespace RenderPipeline {
            VALUE klass_rpm;
            VALUE klass_remote_render_executive;


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
                GetNativeObject<RemoteRenderExecutive>(self)->Push(GetNativeObject<::RenderPipeline>(rp));
                return self;
            }

            void InitRPM() {
                klass_rpm = rb_define_class_under(module, "RenderPipelineM", RenderPipeline::klass);

                //RE
                klass_remote_render_executive = rb_define_class_under(module, "RemoteRenderExecutive", rb_cObject);
                rb_include_module(klass_remote_render_executive, module_release);
                rb_define_alloc_func(klass_remote_render_executive, RefObjNew<::RemoteRenderExecutive>);
                rb_define_method(klass_remote_render_executive, "initialize", (rubyfunc)RE_initialize, 3);
                rb_define_method(klass_remote_render_executive, "reset_fps", (rubyfunc)RE_reset_fps, 1);
                rb_define_method(klass_remote_render_executive, "push", (rubyfunc)RE_push, 1);
                rb_define_method(klass_remote_render_executive, "terminate", (rubyfunc)RE_terminate, 0);
            }
        }
    }
}