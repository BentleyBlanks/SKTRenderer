struct VOut
{
  float4 position : SV_POSITION;
  float3 positionw:POSITIONW;
  float3 normal:NORMAL;
  float3 tangent : TANGENT;
  float2 tex : TEXCOORD;
  float2 rec : POSITIONP;
};


[maxvertexcount(12)]
void GsMain(triangle VOut gin[3],inout TriangleStream<VOut> stream)
{
  for(int i=0;i<3;++i)
  {
    VOut v1;
    v1.position = gin[i].position;
    v1.normal = gin[i].normal;
    v1.positionw = gin[i].positionw;
    v1.tex = gin[i].tex;
    v1.tangent = gin[i].tangent;
    v1.rec = gin[i].rec;
    stream.Append(v1);
  }

  /*
http://ogldev.atspace.co.uk/www/tutorial26/tutorial26.html
http://www.rastertek.com/dx11tut20.html
http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
http://www.slideshare.net/Mark_Kilgard/geometryshaderbasedbumpmappingsetup
http://www.geeks3d.com/20130122/normal-mapping-without-precomputed-tangent-space-vectors/
http://learnopengl.com/#!Advanced-Lighting/Normal-Mapping
*/

  stream.RestartStrip();
}
