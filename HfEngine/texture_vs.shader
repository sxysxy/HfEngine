struct vs_output {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

vs_output main(float4 pos : POSITION, float4 tex : TEXCOORD) {
    vs_output opt;
    opt.pos = pos;
    opt.tex = tex;
    return opt;
}