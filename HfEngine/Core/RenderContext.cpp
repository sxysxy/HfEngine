#include <Core/GDevice.h>
#include <Core/RubyVM.h>
#include <Core/RenderContext.h>

HFENGINE_NAMESPACE_BEGIN

void Shader::CreateFromString(const std::string& code, const std::string& entry) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(code.c_str(), code.length(), 0, 0, 0, 
        entry.c_str(), SHADER_COMPILE_TOKEN[shader_type]
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

void Shader::CreateFromFile(const std::wstring& filename, const std::string& entry) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;

    HRESULT hr = D3DX11CompileFromFileW(filename.c_str(), nullptr, nullptr, entry.c_str(),
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
        //native_vshader->AddRef();
        native_shader = native_vshader;
    }
    else if (shader_type == GEOMETRY_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreateGeometryShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_gshader);
        //native_gshader->AddRef();
        native_shader = native_gshader;
    }
    else if (shader_type == PIXEL_SHADER) {
        hr = GDevice::GetInstance()->native_device->CreatePixelShader(compiled_byte_code->GetBufferPointer(),
            compiled_byte_code->GetBufferSize(), 0, &native_pshader);
        //native_pshader->AddRef();
        native_shader = native_pshader;
    }
    if (FAILED(hr))
        THROW_ERROR_CODE(std::runtime_error, "Fail to Create VertexShader, Error code:", hr);
}

void Shader::CreateFromBinary(const void* ptr, size_t length) {
    try {
        D3D10CreateBlob(length, &compiled_byte_code);
        memcpy(compiled_byte_code->GetBufferPointer(), ptr, length);
        auto device = GDevice::GetInstance()->native_device.Get();
        if(shader_type == VERTEX_SHADER) {
            device->CreateVertexShader(ptr, length, 0, &native_vshader);
        }
        else if (shader_type == GEOMETRY_SHADER) {
            device->CreateGeometryShader(ptr, length, 0, &native_gshader);
        }
        else if (shader_type == PIXEL_SHADER) {
            device->CreatePixelShader(ptr, length, 0, &native_pshader);
        }
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

void GBuffer::Initialize(UINT usage, UINT flag, size_t size, const void *init_data) {
    D3D11_BUFFER_DESC bd;
    RtlZeroMemory(&bd, sizeof bd);
    bd.BindFlags = flag;
    bd.Usage = (D3D11_USAGE)usage;
    bd.ByteWidth = (UINT)size;
    D3D11_SUBRESOURCE_DATA data;
    if (init_data) {
        RtlZeroMemory(&data, sizeof data);
        data.pSysMem = init_data;
    }
    HRESULT hr = GDevice::GetInstance()->native_device->CreateBuffer(&bd, 
        init_data ? &data : nullptr, &native_buffer);
    if (FAILED(hr))
        THROW_ERROR_CODE(std::runtime_error, "Failed to create D3DBuffer, Error code:", hr);
    _size = (int)size;
}

thread_local RClass* ClassShader, *ClassVertexShader, *ClassGeometryShader, *ClassPixelShader;
thread_local RClass* ClassRenderContext;
thread_local RClass* ClassGBuffer, *ClassVertexBuffer, *ClassIndexBuffer, *ClassConstantBuffer;

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
method: HEG::Shader#from_string(code : String, entry = "main" : String) -> self
note: Create Shader from string of code.
*/
static mrb_value ClassShader_from_string(mrb_state* mrb, mrb_value self) {
    mrb_value code;
    const char* entry = "main";
    if(mrb_get_argc(mrb) == 1) {
        mrb_get_args(mrb, "S", &code);
    }
    else {
        mrb_value entry_obj;
        mrb_get_args(mrb, "SS", &code, &entry_obj);
        entry = RSTRING_PTR(entry_obj);
    }
    try {
        GetNativeObject<Shader>(self)->CreateFromString(RSTRING_PTR(code), entry);
    }
    catch (std::exception& e) {
        mrb_raise(mrb, mrb->eStandardError_class, e.what());
    }
    return self;
}

/*[DOCUMENT]
method: HEG::Shader#from_file(filename : String, entry = "main" : String) -> self
note: Create Shader from file named filename
*/
static mrb_value ClassShader_from_file(mrb_state* mrb, mrb_value self) {
    mrb_value filename;
    const char* entry = "main";
    if (mrb_get_argc(mrb) == 1) {
        mrb_get_args(mrb, "S", &filename);
    }
    else {
        mrb_value entry_obj;
        mrb_get_args(mrb, "SS", &filename, &entry_obj);
        entry = RSTRING_PTR(entry_obj);
    }
    std::wstring filenamew;
    U8ToU16(RSTRING_PTR(filename), filenamew);
    try {
        GetNativeObject<Shader>(self)->CreateFromFile(filenamew, entry);
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

/*[DOCUMENT]
method: HEG::Shader#byte_code -> byte_code : String
note: Get compiled byte code
*/
static mrb_value ClassShader_byte_code(mrb_state* mrb, mrb_value self) {
    auto s = GetNativeObject<Shader>(self);
    size_t size = s->compiled_byte_code->GetBufferSize();
    mrb_value b = mrb_str_new_capa(mrb, size);
    mrb_str_resize(mrb, b, size);
    memcpy(RSTRING_PTR(b), s->compiled_byte_code->GetBufferPointer(), size);
    return b;
}

static mrb_data_type ClassVertexBufferDataType = mrb_data_type{ "VertexBuffer", [](mrb_state* mrb, void* ptr) {
    static_cast<GBuffer*>(ptr)->SubRefer();
} };
static mrb_data_type ClassIndexBufferDataType = mrb_data_type{ "IndexBuffer", [](mrb_state* mrb, void* ptr) {
    static_cast<GBuffer*>(ptr)->SubRefer();
} };
static mrb_data_type ClassConstantBufferDataType = mrb_data_type{ "ConstantBuffer", [](mrb_state* mrb, void* ptr) {
    static_cast<GBuffer*>(ptr)->SubRefer();
} };
/*[DOCUMENT]
method: HEG::VertexBuffer::new(num_of_vertex, sizeof_each_vertex, init_data(optional)) -> vb : VertexBuffer
note: Create a vertex buffer using init_data(if given)
*/
static mrb_value ClassVertexBuffer_new(mrb_state* mrb, mrb_value klass) {
    mrb_int num_of_vertex, sizeof_each_vertex;
    mrb_value data;
    void* pdata;
    mrb_int argc = mrb_get_argc(mrb);
    if (argc == 3) {
        mrb_get_args(mrb, "iiS", &num_of_vertex, &sizeof_each_vertex, &data);
        pdata = RSTRING_PTR(data);
    }
    else if (argc == 2) {
        mrb_get_args(mrb, "ii", &num_of_vertex, &sizeof_each_vertex);
        pdata = nullptr;
    }
    else {
        mrb_raise(mrb, mrb->eStandardError_class, "VertexBuffer::new: Wrong number of arguments(expecting 2 or 3)");
        return mrb_value();
    }
    try {
        GBuffer* bf = new VertexBuffer(sizeof_each_vertex, num_of_vertex, pdata);
        bf->AddRefer();
        return mrb_obj_value(mrb_data_object_alloc(mrb, ClassVertexBuffer, bf, &ClassVertexBufferDataType));
    }
    catch (std::exception & e) {
        mrb_raise(mrb, mrb->eStandardError_class, (std::string("VertexBuffer::new: Failed to create Vertex Buffer: ") + e.what()).c_str());
        return mrb_value();
    }
}

/*[DOCUMENT]
method: HEG::IndexBuffer::new(num_of_index, init_data(optional)) -> ib : IndexBuffer
note: Create an index buffer using init_data(if given)
*/
static mrb_value ClassIndexBuffer_new(mrb_state* mrb, mrb_value klass) {
    mrb_int num_of_index;
    mrb_value data;
    void* pdata;
    mrb_int argc = mrb_get_argc(mrb);
    if (argc == 2) {
        mrb_get_args(mrb, "iS", &num_of_index, &data);
        pdata = RSTRING_PTR(data);
    }
    else if (argc == 1) {
        mrb_get_args(mrb, "i", &num_of_index);
        pdata = nullptr;
    }
    else {
        mrb_raise(mrb, mrb->eStandardError_class, "IndexBuffer::new: Wrong number of arguments(expecting 2 or 3)");
        return mrb_value();
    }
    try {
        GBuffer* bf = new IndexBuffer(num_of_index, (int32_t*)pdata);
        bf->AddRefer();
        return mrb_obj_value(mrb_data_object_alloc(mrb, ClassIndexBuffer, bf, &ClassIndexBufferDataType));
    }
    catch (std::exception & e) {
        mrb_raise(mrb, mrb->eStandardError_class, (std::string("IndexBuffer::new: Failed to create Index Buffer: ") + e.what()).c_str());
        return mrb_value();
    }
}


/*[DOCUMENT]
method: HEG::ConstantBuffer::new(size_in_bytes, init_data(optional)) -> cb : Constant
note: Create a constant buffer using init_data(if given)
*/
static mrb_value ClassConstantBuffer_new(mrb_state* mrb, mrb_value klass) {
    mrb_int size_in_bytes;
    mrb_value data;
    void* pdata;
    mrb_int argc = mrb_get_argc(mrb);
    if (argc == 2) {
        mrb_get_args(mrb, "iS", &size_in_bytes, &data);
        pdata = RSTRING_PTR(data);
    }
    else if (argc == 1) {
        mrb_get_args(mrb, "i", &size_in_bytes);
        pdata = nullptr;
    }
    else {
        mrb_raise(mrb, mrb->eStandardError_class, "ConstantBuffer::new: Wrong number of arguments(expecting 2 or 3)");
        return mrb_value();
    }
    try {
        GBuffer* bf = new ConstantBuffer(size_in_bytes, (int32_t*)pdata);
        bf->AddRefer();
        return mrb_obj_value(mrb_data_object_alloc(mrb, ClassConstantBuffer, bf, &ClassConstantBufferDataType));
    }
    catch (std::exception & e) {
        mrb_raise(mrb, mrb->eStandardError_class, (std::string("ConstantBuffer::new: Failed to create Constant Buffer: ") + e.what()).c_str());
        return mrb_value();
    }
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
    mrb_int len1 = RARRAY_LEN(names);
    mrb_int len2 = RARRAY_LEN(fmts);
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
        context->SetInputLayout(ns.data(), fs.data(), (int)len1);
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

/*[DOCUMENT]
method: HEG::RenderContext#vbuffer(vb : VertexBuffer) -> self
note: Set Vertex Buffer
*/
mrb_value ClassRenderContext_vbuffer(mrb_state* mrb, mrb_value self) {
    mrb_value vb_obj;
    mrb_get_args(mrb, "o", &vb_obj);
    if (!mrb_obj_is_kind_of(mrb, vb_obj, ClassVertexBuffer)) {
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#vbuffer: argument is expected to be a Vertex Buffer");
        return self;
    }
    GetNativeObject<RenderContext>(self)->SetVertexBuffer(GetNativeObject<VertexBuffer>(vb_obj));
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#ibuffer(vb : IndexBuffer) -> self
note: Set Index Buffer
*/
mrb_value ClassRenderContext_ibuffer(mrb_state* mrb, mrb_value self) {
    mrb_value ib_obj;
    mrb_get_args(mrb, "o", &ib_obj);
    if (!mrb_obj_is_kind_of(mrb, ib_obj, ClassIndexBuffer)) {
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#ibuffer: argument is expected to be a Index Buffer");
        return self;
    }
    GetNativeObject<RenderContext>(self)->SetIndexBuffer(GetNativeObject<IndexBuffer>(ib_obj));
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#viewport(topx : Integer, topy : Integer, width : Integer, height : Integer) -> self
note: Set the viewport
*/
mrb_value ClassRenderContext_viewport(mrb_state* mrb, mrb_value self) {
    mrb_int argc = mrb_get_argc(mrb);
    mrb_int topx, topy, w, h;
    mrb_get_args(mrb, "iiii", &topx, &topy, &w, &h);
    GetNativeObject<RenderContext>(self)->SetViewport((UINT)topx, (UINT)topy, (UINT)w, (UINT)h);
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#render -> self
note: Execute render command on GDevice. It will acquire GDevice lock
*/
mrb_value ClassRenderContext_render(mrb_state* mrb, mrb_value self) {
    GetNativeObject<RenderContext>(self)->Render();
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#target(canvas : Canvas) -> self
note: Set the rendering target
*/
mrb_value ClassRenderContext_target(mrb_state* mrb, mrb_value self) {
    mrb_value cv_obj;
    mrb_get_args(mrb, "o", &cv_obj);
    if (!mrb_obj_is_kind_of(mrb, cv_obj, ClassCanvas)) {
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#target: argument is expected to be a Canvas");
        return self;
    }
    GetNativeObject<RenderContext>(self)->SetRenderTarget(GetNativeObject<Canvas>(cv_obj));
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#clear | HEG::RenderContext#clear(r, g, b, a) -> self
note: Clear render target using given color
*/
mrb_value ClassRenderContext_clear(mrb_state* mrb, mrb_value self) {
    mrb_int argc = mrb_get_argc(mrb);
    if (argc == 0) GetNativeObject<RenderContext>(self)->ClearTarget();
    else if (argc == 4) {
        mrb_value r, g, b, a;
        mrb_get_args(mrb, "ffff", &r, &g, &b, &a);
        GetNativeObject<RenderContext>(self)->ClearTarget((float)mrb_float(r),
            (float)mrb_float(g),
            (float)mrb_float(b),
            (float)mrb_float(a));
    }
    else {
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#clear requires 0 or 4 floats number arguments to work");
    }
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#resource(stage : SHADER_TYPE, slot : Fixnum, canvas : Canvas) -> self 
note: Bind shader resource to slot
*/
mrb_value ClassRenderContext_resource(mrb_state* mrb, mrb_value self) {
    mrb_int stage, slot;
    mrb_value canvas;
    mrb_get_args(mrb, "iio", &stage, &slot, &canvas);
    if (!mrb_obj_is_kind_of(mrb, canvas, ClassCanvas)) {
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#resource(stage, slot, canvas): argument 3 is expected to be a Canvas");
        return self;
    }
    GetNativeObject<RenderContext>(self)->SetShaderResource((SHADER_TYPE)stage, (int)slot,
        GetNativeObject<Canvas>(canvas));
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#cbuffer(stage : SHADER_TYPE, slot : Fixnum, cb : ConstantBuffer) -> self
note: Bind constant buffer to slot
*/
mrb_value ClassRenderContext_cbuffer(mrb_state* mrb, mrb_value self) {
    mrb_int stage, slot;
    mrb_value cb;
    mrb_get_args(mrb, "iio", &stage, &slot, &cb);
    if (!mrb_obj_is_kind_of(mrb, cb, ClassConstantBuffer)) {
        mrb_raise(mrb, mrb->eStandardError_class, "RenderContext#cbuffer(stage, slot, cb): argument 3 is expected to be a ConstantBuffer");
        return self;
    }
    GetNativeObject<RenderContext>(self)->SetConstantBuffer((SHADER_TYPE)stage, (int)slot,
        GetNativeObject<ConstantBuffer>(cb));
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#draw(topology, start_vertex_pos, count) -> self
note: Make a draw call to draw count vertexes from start_vertex_pos in vertex buffer
*/
mrb_value ClassRenderContext_draw(mrb_state* mrb, mrb_value self) {
    mrb_int topology, start_pos, count;
    mrb_get_args(mrb, "iii", &topology, &start_pos, &count);
    GetNativeObject<RenderContext>(self)->Draw((D3D11_PRIMITIVE_TOPOLOGY)topology, (UINT)start_pos, (UINT)count);
    return self;
}

/*[DOCUMENT]
method: HEG::RenderContext#draw_index(topology, start_index_pos, count) -> self
note: Make a draw call to draw count vertexes, vertexes' indexes are specified by index buffer
*/
mrb_value ClassRenderContext_draw_index(mrb_state* mrb, mrb_value self) {
    mrb_int topology, start_pos, count;
    mrb_get_args(mrb, "iii", &topology, &start_pos, &count);
    GetNativeObject<RenderContext>(self)->DrawIndex((D3D11_PRIMITIVE_TOPOLOGY)topology, (UINT)start_pos, (UINT)count);
    return self;
}

bool InjectRenderContextExtension() {
    mrb_state* mrb = currentRubyVM->GetRuby();
    RClass* Object = mrb->object_class;
    RClass* HEG = mrb_define_module(mrb, "HEG");
    ClassShader = mrb_define_class_under(mrb, HEG, "Shader", Object);
    mrb_define_method(mrb, ClassShader, "byte_code", ClassShader_byte_code, MRB_ARGS_NONE());
    ClassVertexShader = mrb_define_class_under(mrb, HEG, "VertexShader", ClassShader);
    ClassGeometryShader = mrb_define_class_under(mrb, HEG, "GeometryShader", ClassShader);
    ClassPixelShader = mrb_define_class_under(mrb, HEG, "PixelShader", ClassShader);
    mrb_define_class_method(mrb, ClassVertexShader, "new", ClassVertexShader_new, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, ClassGeometryShader, "new", ClassGeometryShader_new, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, ClassPixelShader, "new", ClassPixelShader_new, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassShader, "from_string", ClassShader_from_string, MRB_ARGS_ARG(1, 1));
    mrb_define_method(mrb, ClassShader, "from_file", ClassShader_from_file, MRB_ARGS_ARG(1, 1));
    mrb_define_method(mrb, ClassShader, "from_binary", ClassShader_from_binary, MRB_ARGS_REQ(1));
    /*[DOCUMENT]
    constant: Shader::VERTEX
    note: Vertex Shader Flag
    */
    mrb_define_const(mrb, ClassShader, "VERTEX", mrb_fixnum_value(VERTEX_SHADER));
    /*[DOCUMENT]
    constant: Shader::GEOMETRY
    note: Geometry Shader Flag
    */
    mrb_define_const(mrb, ClassShader, "GEOMETRY", mrb_fixnum_value(GEOMETRY_SHADER));
    /*[DOCUMENT]
    constant: Shader::PIXEL
    note: Pixel Shader Flag
    */
    mrb_define_const(mrb, ClassShader, "PIXEL", mrb_fixnum_value(PIXEL_SHADER));

    ClassRenderContext = mrb_define_class_under(mrb, HEG, "RenderContext", Object);
    mrb_define_class_method(mrb, ClassRenderContext, "new", ClassRenderContext_new, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassRenderContext, "layout", ClassRenderContext_layout, MRB_ARGS_REQ(2));
    /*[DOCUMENT]
    constant: FORMAT_FLOAT4
    note: Format of 4 float numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_FLOAT4", mrb_fixnum_value(DXGI_FORMAT_R32G32B32A32_FLOAT));
    /*[DOCUMENT]
    constant: FORMAT_SINT4
    note: Format of 4 singed integer numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_SINT4", mrb_fixnum_value(DXGI_FORMAT_R32G32B32A32_SINT));
    /*[DOCUMENT]
    constant: FORMAT_UINT4
    note: Format of 4 unsigned integer numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_UINT4", mrb_fixnum_value(DXGI_FORMAT_R32G32B32A32_UINT));
    /*[DOCUMENT]
    constant: FORMAT_FLOAT3
    note: Format of 3 float numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_FLOAT3", mrb_fixnum_value(DXGI_FORMAT_R32G32B32_FLOAT));
    /*[DOCUMENT]
    constant: FORMAT_SINT3
    note: Format of 3 signed integer numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_SINT3", mrb_fixnum_value(DXGI_FORMAT_R32G32B32_SINT));
    /*[DOCUMENT]
    constant: FORMAT_UINT3
    note: Format of 3 unsigned integer numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_UINT3", mrb_fixnum_value(DXGI_FORMAT_R32G32B32_UINT));
    /*[DOCUMENT]
    constant: FORMAT_FLOAT2
    note: Format of 2 float numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_FLOAT2", mrb_fixnum_value(DXGI_FORMAT_R32G32_FLOAT));
    /*[DOCUMENT]
    constant: FORMAT_SINT2
    note: Format of 2 signed integer numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_SINT2", mrb_fixnum_value(DXGI_FORMAT_R32G32_SINT));
    /*[DOCUMENT]
    constant: FORMAT_SINT2
    note: Format of 2 unsigned integer numbers.
    */
    mrb_define_const(mrb, HEG, "FORMAT_SINT2", mrb_fixnum_value(DXGI_FORMAT_R32G32_UINT));
    mrb_define_method(mrb, ClassRenderContext, "shader", ClassRenderContext_shader, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassRenderContext, "vbuffer", ClassRenderContext_vbuffer, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassRenderContext, "ibuffer", ClassRenderContext_ibuffer, MRB_ARGS_REQ(1));
    /*[DOCUMENT]
    constant: TOPOLOGY_TRIANGLES
    note: As its name.
    */
    mrb_define_const(mrb, HEG, "TOPOLOGY_TRIANGLES", mrb_fixnum_value(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
    /*[DOCUMENT]
    constant: TOPOLOGY_TRIANGLES_ADJ
    note: As its name.
    */
    mrb_define_const(mrb, HEG, "TOPOLOGY_TRIANGLES_ADJ", mrb_fixnum_value(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ));
    /*[DOCUMENT]
    constant: TOPOLOGY_TRIANGLES_STRIP
    note: As its name.
    */
    mrb_define_const(mrb, HEG, "TOPOLOGY_TRIANGLES_STRIP", mrb_fixnum_value(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));
    /*[DOCUMENT]
    constant: TOPOLOGY_POINTS
    note: As its name.
    */
    mrb_define_const(mrb, HEG, "TOPOLOGY_POINTS", mrb_fixnum_value(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST));
    /*[DOCUMENT]
    constant: TOPOLOGY_LINES
    note: As its name.
    */
    mrb_define_const(mrb, HEG, "TOPOLOGY_LINES", mrb_fixnum_value(D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
    /*[DOCUMENT]
    constant: TOPOLOGY_LINES_ADJ
    note: As its name.
    */
    mrb_define_const(mrb, HEG, "TOPOLOGY_LINES_ADJ", mrb_fixnum_value(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ));
    /*[DOCUMENT]
    constant: TOPOLOGY_LINES_STRIP
    note: As its name.
    */
    mrb_define_const(mrb, HEG, "TOPOLOGY_LINES_STRIP", mrb_fixnum_value(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP));
    mrb_define_method(mrb, ClassRenderContext, "viewport", ClassRenderContext_viewport, MRB_ARGS_REQ(4));
    mrb_define_method(mrb, ClassRenderContext, "target", ClassRenderContext_target, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassRenderContext, "clear", ClassRenderContext_clear, MRB_ARGS_ANY());
    mrb_define_method(mrb, ClassRenderContext, "resource", ClassRenderContext_resource, MRB_ARGS_REQ(3));
    mrb_define_method(mrb, ClassRenderContext, "draw", ClassRenderContext_draw, MRB_ARGS_REQ(3));
    mrb_define_method(mrb, ClassRenderContext, "draw_index", ClassRenderContext_draw_index, MRB_ARGS_REQ(3));
    mrb_define_method(mrb, ClassRenderContext, "render", ClassRenderContext_render, MRB_ARGS_REQ(1));

    ClassGBuffer = mrb_define_class_under(mrb, HEG, "GBuffer", Object);
    ClassVertexBuffer = mrb_define_class_under(mrb, HEG, "VertexBuffer", ClassGBuffer);
    ClassIndexBuffer = mrb_define_class_under(mrb, HEG, "IndexBuffer", ClassGBuffer);
    ClassConstantBuffer = mrb_define_class_under(mrb, HEG, "ConstantBuffer", ClassGBuffer);
    mrb_define_class_method(mrb, ClassVertexBuffer, "new", ClassVertexBuffer_new, MRB_ARGS_ARG(2, 1));
    mrb_define_class_method(mrb, ClassIndexBuffer, "new", ClassIndexBuffer_new, MRB_ARGS_ARG(1, 1));
    mrb_define_class_method(mrb, ClassConstantBuffer, "new", ClassConstantBuffer_new, MRB_ARGS_ARG(1, 1));

    return true;
}

HFENGINE_NAMESPACE_END