struct vs_output{
    float4 pos : SV_POSITION;
};

struct vs_input{
    float4 pos : POSITION;
};

vs_output main(vs_input input){
    vs_output output = (vs_output)input;
    return output;
}