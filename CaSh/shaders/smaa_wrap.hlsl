cbuffer SMAABuffer
{
  matrix m;
  matrix v;
  matrix o;
  float3 sun_dir_m;
  float3 sun_color_m;
  PointLight point_lights[MAX_POINTLIGHT];
  int light_n;
  float3 view_world_pos;
  float roughness;
  float metallic;
  int shading_model;

};