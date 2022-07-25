float4x4 gWorldMatrix;
float4x4 gViewMatrix;
float4x4 gProjectionMatrix;

struct VS_INPUT
{
   float4 mPosition : POSITION; // 여기에서 세미콜론 없으면 에러.
}; // 세미콜론 없으면 "syntax error: unexpected token 'struct'" 에러 난다.

struct VS_OUTPUT
{
   float4 mPosition : POSITION;
};

VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;
   Output.mPosition = mul( Input.mPosition, gWorldMatrix );
   Output.mPosition = mul( Output.mPosition, gViewMatrix );
   Output.mPosition = mul( Output.mPosition, gProjectionMatrix );

   return Output;
}