#include <Core.h>
#include <Core/RenderContext.h>

HFENGINE_NAMESPACE_BEGIN

void Shader::CreateFromString(const std::string& code) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(code.c_str(), code.length(), 0, 0, 0, "main", SHADER_COMPILE_TOKEN[shader_type]
        , 0, 0, 0, &sbuffer, &errmsg, 0);
    if (FAILED(hr)) {
        if (errmsg) {
            std::string msg;
            msg.append("Shader :Compiler Message:\n");
            msg.append((LPCSTR)errmsg->GetBufferPointer());
            throw std::runtime_error(msg);
        }
        else {
            THROW_ERROR_CODE(std::runtime_error, "Fail to Create Shader from string, Error code:", hr);
        }
    }
    compiled_byte_code = sbuffer;
    if (shader_type == VERTEX_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreateVertexShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_vshader);
        native_vshader->AddRef();
        native_shader = native_vshader;
    }
    else if (shader_type == GEOMETRY_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreateGeometryShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_gshader);
        native_gshader->AddRef();
        native_shader = native_gshader;
    }
    else if (shader_type == PIXEL_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreatePixelShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_pshader);
        native_pshader->AddRef();
        native_shader = native_pshader;
    }
    if (FAILED(hr))
        THROW_ERROR_CODE(std::runtime_error, "Fail to Create VertexShader, Error code:", hr);
}

void Shader::CreateFromFile(const std::wstring& filename) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;

    HRESULT hr = D3DX11CompileFromFileW(filename.c_str(), nullptr, nullptr, "main",
        SHADER_COMPILE_TOKEN[shader_type], 0, 0, 0,
        &sbuffer, &errmsg, nullptr); //cause a _com_error,,but why?, it returns S_OK... 

    if (FAILED(hr)) {
        if (errmsg) {
            std::string msg;
            msg.append("Shader :Compiler Message:\n");
            msg.append((LPCSTR)errmsg->GetBufferPointer());
            throw std::runtime_error(msg);
        }
        else {
            THROW_ERROR_CODE(std::runtime_error, "Fail to Create Shader from string, Error code:", hr);
        }
    }
    compiled_byte_code = sbuffer;
    if (shader_type == VERTEX_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreateVertexShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_vshader);
        native_vshader->AddRef();
        native_shader = native_vshader;
    }
    else if (shader_type == GEOMETRY_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreateGeometryShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_gshader);
        native_gshader->AddRef();
        native_shader = native_gshader;
    }
    else if (shader_type == PIXEL_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreatePixelShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_pshader);
        native_pshader->AddRef();
        native_shader = native_pshader;
    }
    if (FAILED(hr))
        THROW_ERROR_CODE(std::runtime_error, "Fail to Create VertexShader, Error code:", hr);
}

void Shader::CreateFromBinary(const void* ptr, size_t length) {
    try {
        D3D10CreateBlob(length, &compiled_byte_code);
        memcpy(compiled_byte_code->GetBufferPointer(), ptr, length);
        GDevice::GetInstance()->native_device->CreateGeometryShader(ptr, length, 0, &native_gshader);
    }
    catch (std::exception) {
        native_shader = nullptr;
        if (shader_type == VERTEX_SHADER)native_vshader = nullptr;
        else if (shader_type == GEOMETRY_SHADER)native_gshader = nullptr;
        else if (shader_type == PIXEL_SHADER)native_pshader = nullptr;
    }
}

void RenderContext::SetInputLayout(const std::string* idents,
    const DXGI_FORMAT* formats, int count) {
#ifdef _DEBUG
    if (!vshader) {
        throw std::runtime_error("No vertex shader provided, you can not set input layout now");
        return;
    }
#endif
    std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
    for (int i = 0; i < count; i++) {
        ied.push_back(D3D11_INPUT_ELEMENT_DESC{ idents[i].c_str(), 0, formats[i], 0,
            D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 });
    }
    ComPtr<ID3D11InputLayout> native_input_layout;
    HRESULT hr = S_FALSE;
    if (FAILED(hr = GDevice::GetInstance()->native_device->CreateInputLayout(ied.data(), count, 
        vshader->compiled_byte_code->GetBufferPointer(),
        vshader->compiled_byte_code->GetBufferSize(), 
        &native_input_layout))) {
        THROW_ERROR_CODE(std::runtime_error, "Fail to Create InputLayout, Error code:", hr);
    }
    native_context->IASetInputLayout(native_input_layout.Get());
}

thread_local RClass* ClassShader, * ClassVertexShader, * ClassGeometryShader, * ClassPixelShader;
thread_local RClass* ClassRenderContext;

static mrb_data_type ClassShaderDataType = mrb_data_type{ "Shader", [](mrb_state* mrb, void* ptr) {
    static_cast<Shader*>(ptr)->SubRefer();
} };
static mrb_data_type ClassVertexShaderDataType = mrb_data_type{ "VertexShader", [](mrb_state* mrb, void* ptr) {
    static_cast<Shader*>(ptr)->SubRefer();
} };
static mrb_data_type ClassGeometryShaderDataType = mrb_data_type{ "GeometryShader", [](mrb_state* mrb, void* ptr) {
    static_cast<Shader*>(ptr)->SubRefer();
} };
static mrb_data_type ClassPixelShaderDataType = mrb_data_type{ "PixelShader", [](mrb_state* mrb, void* ptr) {
    static_cast<Shader*>(ptr)->SubRefer();
} };

/*[DOCUMENT]
method: HEG::VertexShader::new -> shader : VertexShader
note: Create a new VertexShader object
*/
static mrb_value ClassVertexShader_new(mrb_state* mrb, mrb_value klass) {
    Shader* s = new VertexShader();
    s->AddRefer();
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassVertexShader, s, &ClassVertexShaderDataType));
}
/*[DOCUMENT]
method: HEG::GeometryShader::new -> shader : GeometryShader
note: Create a new GeometryShader object
*/
static mrb_value ClassGeometryShader_new(mrb_state* mrb, mrb_value klass) {
    Shader* s = new GeometryShader();
    s->AddRefer();
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassVertexShader, s, &ClassGeometryShaderDataType));
}
/*[DOCUMENT]
method: HEG::PixelShader::new -> shader : PixelShader
note: Create a new PixelShader object
*/
static mrb_value ClassPixelShader_new(mrb_state* mrb, mrb_value klass) {
    Shader* s = new PixelShader();
    s->AddRefer();
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassVertexShader, s, &ClassPixelShaderDataType));
}

/*[DOCUMENT]
method: HEG::Shader#from_string(code : String) -> self
note: Create Shader from string of code.
*/
static mrb_value ClassShader_from_string(mrb_state* mrb, mrb_value self) {
    mrb_value code;
    mrb_get_args(mrb, "S", &code);
    try {
        GetNativeObject<Shader>(self)->CreateFromString(RSTRING_PTR(code));
    }
    catch (std::exception& e) {
        mrb_raise(mrb, mrb->eStandardError_class, e.what());
    }
    return self;
}

/*[DOCUMENT]
method: HEG::Shader#from_file(filename : String) -> self
note: Create Shader from file named filename
*/
static mrb_value ClassShader_from_file(mrb_state* mrb, mrb_value self) {
    mrb_value filename;
    mrb_get_args(mrb, "S", &filename);
    std::wstring filenamew;
    U8ToU16(RSTRING_PTR(filename), filenamew);
    try {
        GetNativeObject<Shader>(self)->CreateFromFile(filenamew);
    }
    catch (std::exception & e) {
        mrb_raise(mrb, mrb->eStandardError_class, e.what());
    }
    return self;
}

/*[DOCUMENT]
method: HEG::Shader#from_binary(byte_code : String) -> self
note: Create Shader from compiled byte code
*/
static mrb_value ClassShader_from_binary(mrb_state* mrb, mrb_value self) {
    mrb_value code;
    mrb_get_args(mrb, "S", &code);
    try {
        GetNativeObject<Shader>(self)->CreateFromBinary(RSTRING_PTR(code), RSTRING_LEN(code));
    }
    catch (std::exception & e) {
        mrb_raise(mrb, mrb->eStandardError_class, e.what());
    }
    return self;
}


static mrb_data_type ClassRenderContextDataType = mrb_data_type{ "RenderContext", [](mrb_state* mrb, void* ptr) {
    static_cast<RenderContext*>(ptr)->SubRefer();
} };
/*[DOCUMENT]
method: HEG::RenderContext::new -> context : RenderContext
note: Create a new render context
*/
static mrb_value ClassRenderContext_new(mrb_state* mrb, mrb_value klass) {
    RenderContext* context = new RenderContext();
    context->AddRefer();
    context->Initialize();
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassRenderContext, context, &ClassRenderContextDataType));
}

/*[DOCUMENT]
method: HEG::RenderContext#layout -> self
note: Set the input layout
*/
static mrb_value ClassRenderContext_layout(mrb_state* mrb, mrb_value self) {
    mrb_value names, fmts;
    mrb_get_args(mrb, "AA", &names, &fmts);
    
    mrb_value* pnames = RARRAY_PTR(names);
    mrb_value* pfmts = RARRAY_PTR(fmts);
    int len1 = RARRAY_LEN(names);
    int len2 = RARRAY_LEN(fmts);
    if (len1 != len2)
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#layout: Array lengths do not match");
    std::vector<std::string> ns;
    std::vector<DXGI_FORMAT> fs;
    for (int i = 0; i < len1; i++) {
        ns.push_back(RSTRING_PTR(pnames[i]));
        fs.push_back((DXGI_FORMAT)mrb_fixnum(pfmts[i]));
    }
    auto context = GetNativeObject<RenderContext>(self);
    auto device = GDevice::GetInstance();
    try {
        context->SetInputLayout(ns.data(), fs.data(), len1);
    }
    catch (std::runtime_error re) {
        mrb_raise(mrb, mrb->eStandardError_class, re.what());
    }
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#shader(shader : Shader) -> self
note: Set shader(Auto-detect shader's type)
*/
mrb_value ClassRenderContext_shader(mrb_state* mrb, mrb_value self) {
    mrb_value shader_obj;
    mrb_get_args(mrb, "o", &shader_obj);
    if (!mrb_obj_is_kind_of(mrb, shader_obj, ClassShader)) {
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#shader: argument is expected to be a Shader");
        return self;
    }
    GetNativeObject<RenderContext>(self)->SetShader(GetNativeObject<Shader>(shader_obj));
    return self;
}

bool InjectRenderContextExtension() {
    mrb_state* mrb = currentRubyVM->GetRuby();
    RClass* Object = mrb->object_class;
    RClass* HEG = mrb_define_module(mrb, "HEG");
    ClassShader = mrb_define_class_under(mrb, HEG, "Shader", Object);
    ClassVertexShader = mrb_define_class_under(mrb, HEG, "VertexShader", ClassShader);
    ClassGeometryShader = mrb_define_class_under(mrb, HEG, "GeometryShader", ClassShader);
    ClassPixelShader = mrb_define_class_under(mrb, HEG, "PixelShader", ClassShader);
    mrb_define_class_method(mrb, ClassVertexShader, "new", ClassVertexShader_new, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, ClassGeometryShader, "new", ClassGeometryShader_new, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, ClassPixelShader, "new", ClassPixelShader_new, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassShader, "from_string", ClassShader_from_string, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassShader, "from_file", ClassShader_from_file, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassShader, "from_binary", ClassShader_from_binary, MRB_ARGS_REQ(1));

    ClassRenderContext = mrb_define_class_under(mrb, HEG, "RenderContext", Object);
    mrb_define_class_method(mrb, ClassRenderContext, "new", ClassRenderContext_new, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassRenderContext, "layout", ClassRenderContext_layout, MRB_ARGS_REQ(2));
    mrb_define_method(mrb, ClassRenderContext, "shader", ClassRenderContext_shader, MRB_ARGS_REQ(1));
    return true;
}

HFENGINE_NAMESPACE_END