#include <Core/RubyVM.h>
#include <Core/Transform.h>

HFENGINE_NAMESPACE_BEGIN

static mrb_data_type ClassTransformDataType = mrb_data_type{ "Transform", [](mrb_state* mrb, void* ptr) {
    delete ptr;
} };

/*[DOCUMENT]
method: HEG::Transform::new -> trans : Transform
note: New a transform object
*/
static mrb_value ClassTransform_new(mrb_state* mrb, mrb_value self) {
    auto m = new Transform();
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassTransform, m, &ClassTransformDataType));
}

/*[DOCUMENT]
method: HEG::Transform#data -> data : String
note: Return the packed floats data
*/
static mrb_value ClassTransform_data(mrb_state* mrb, mrb_value self) {
    auto cur = GetNativeObject<Transform>(self);
    mrb_value s = mrb_str_new_capa(mrb, sizeof(float) * 16);
    mrb_str_resize(mrb, s, sizeof(float) * 16);
    //memcpy(RSTRING_PTR(s), cur->data(), sizeof(float) * 16);
    using float4 = float[4];
    float4* pdest = (float4*)RSTRING_PTR(s);
    for (int i = 0; i < 4; i++)for (int j = 0; j < 4; j++) {
        pdest[j][i] = cur->matrix.m[i][j];
    }
    return s;
}

/*[DOCUMENT]
method: HEG::Transform#translate(tx, ty, tz) -> self
note: Attach translate transform to self
*/
static mrb_value ClassTransform_translate(mrb_state* mrb, mrb_value self) {
    auto cur = GetNativeObject<Transform>(self);
    mrb_value tx, ty, tz;
    mrb_get_args(mrb, "fff", &tx, &ty, &tz);
    cur->translate((float)mrb_float(tx), (float)mrb_float(ty), (float)mrb_float(tz));
    return self;
}

/*[DOCUMENT]
method: HEG::Transform#scale(sx, sy, sz) -> self
note: Attach scale transform to self
*/
static mrb_value ClassTransform_scale(mrb_state* mrb, mrb_value self) {
    auto cur = GetNativeObject<Transform>(self);
    mrb_value sx, sy, sz;
    mrb_get_args(mrb, "fff", &sx, &sy, &sz);
    cur->scale((float)mrb_float(sx), (float)mrb_float(sy), (float)mrb_float(sz));
    return self;
}

/*[DOCUMENT]
method: HEG::Transform#rotate(axis : Array of floats, angle) -> self
note: Attach rotate transform to self, rotate angle around axis, clockwise when looking direction is the same as axis's. 
*/
static mrb_value ClassTransform_rotate(mrb_state* mrb, mrb_value self) {
    auto cur = GetNativeObject<Transform>(self);
    mrb_value axis, angle;
    mrb_get_args(mrb, "Af", &axis, &angle);
    if (RARRAY_LEN(axis) < 3) {
        mrb_raise(mrb, mrb->eStandardError_class, "Transform#rotate(axis, angle) : axis should contains three float numbers");
        return self;
    }
    mrb_value* pa = RARRAY_PTR(axis);
    float a[] = { (float)mrb_float(pa[0]), (float)mrb_float(pa[1]), (float)mrb_float(pa[2]) };
    cur->rotate(a, (float)mrb_float(angle));
    return self;
}

/*[DOCUMENT]
method: HEG::Transform#view(eye_pos : Array of floats, target_pos : Array of floats, up_dir : Array of floats) -> self
note: Attach view transform to self
*/
static mrb_value ClassTransform_view(mrb_state* mrb, mrb_value self) {
    auto cur = GetNativeObject<Transform>(self);
    mrb_value ep, tp, ud;
    mrb_get_args(mrb, "AAA", &ep, &tp, &ud);
    if (RARRAY_LEN(ep) < 3 || RARRAY_LEN(tp) < 3 || RARRAY_LEN(ud) < 3) {
        mrb_raise(mrb, mrb->eStandardError_class, "Transform#view : All parameters should contains 3 float numbers");
        return self;
    }
    const mrb_value* pep = RARRAY_PTR(ep);
    float epa[] = { (float)mrb_float(pep[0]), (float)mrb_float(pep[1]), (float)mrb_float(pep[2]) };
    const mrb_value* ptp = RARRAY_PTR(tp);
    float tpa[] = { (float)mrb_float(ptp[0]), (float)mrb_float(ptp[1]), (float)mrb_float(ptp[2]) };
    const mrb_value* pud = RARRAY_PTR(ud);
    float uda[] = { (float)mrb_float(pud[0]), (float)mrb_float(pud[1]), (float)mrb_float(pud[2]) };
    cur->view(epa, tpa, uda);
    return self;
}

/*[DOCUMENT]
method: HEG::Transform#perspective(fov : Float, aspect_ratio : Float, z_near : Float, z_Far : Float) -> self
note: Attach perspective transform to self
*/
static mrb_value ClassTransform_perspective(mrb_state* mrb, mrb_value self) {
    auto cur = GetNativeObject<Transform>(self);
    mrb_value fov, ar, zn, zf;
    mrb_get_args(mrb, "ffff", &fov, &ar, &zn, &zf);
    cur->perspective((float)mrb_float(fov), (float)mrb_float(ar), (float)mrb_float(zn), (float)mrb_float(zf));
    return self;
}

thread_local RClass* ClassTransform;
bool InjectTransformExtension() {
    mrb_state* mrb = currentRubyVM->GetRuby();
    RClass* ClassObject = mrb->object_class;
    RClass* HEG = mrb_define_module(mrb, "HEG");
    ClassTransform = mrb_define_class_under(mrb, HEG, "Transform", mrb->object_class);
    mrb_define_class_method(mrb, ClassTransform, "new", ClassTransform_new, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassTransform, "data", ClassTransform_data, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassTransform, "translate", ClassTransform_translate, MRB_ARGS_REQ(3));
    mrb_define_method(mrb, ClassTransform, "scale", ClassTransform_scale, MRB_ARGS_REQ(3));
    mrb_define_method(mrb, ClassTransform, "rotate", ClassTransform_rotate, MRB_ARGS_REQ(2));
    mrb_define_method(mrb, ClassTransform, "view", ClassTransform_view, MRB_ARGS_REQ(3));
    mrb_define_method(mrb, ClassTransform, "perspective", ClassTransform_perspective, MRB_ARGS_REQ(4));
    return true;
}

HFENGINE_NAMESPACE_END