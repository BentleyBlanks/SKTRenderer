#include "Renderer.h"
#include "logger.h"
#include "Assertion.h"

int constantTypeSizes[CONSTANT_TYPE_COUNT] = {
  sizeof(float),
  sizeof(RBVector2),
  sizeof(RBVector3),
  sizeof(RBVector4),
  sizeof(int),
  sizeof(int) * 2,
  sizeof(int) * 3,
  sizeof(int) * 4,
  sizeof(int),
  sizeof(int) * 2,
  sizeof(int) * 3,
  sizeof(int) * 4,
  (size_t)((float)sizeof(RBMatrix)*0.25f),
  (size_t)((float)sizeof(RBMatrix)*0.5625f),
  sizeof(RBMatrix),
};


Renderer::Renderer(){
  nImageUnits = 1;
  nMRTs = 1;
  maxAnisotropic = 1;

  viewportWidth = 0;
  viewportHeight = 0;

  fontBuffer = NULL;
  fontBufferCount = 0;

#ifdef PROFILE
  profileFrame = false;
  profileReset = true;
  profileString[0] = '\0';
#endif
}

Renderer::~Renderer(){
  //	delete textureLod;

  delete fontBuffer;
}

void Renderer::resetToDefaults()
{
  //reset current state
  currentShader = SHADER_NONE;
  currentVertexFormat = VF_NONE;
  for (uint i = 0; i < MAX_VERTEXSTREAM; i++){
    currentVertexBuffers[i] = VB_NONE;
    currentOffsets[i] = 0;
  }
  currentIndexBuffer = IB_NONE;

  currentDepthState = DS_NONE;
  currentStencilRef = 0;
  currentBlendState = BS_NONE;
  currentSampleMask = ~0;
  currentRasterizerState = RS_NONE;


  currentColorRT[0] = FB_COLOR;
  currentColorRTSlice[0] = NO_SLICE;
  for (uint i = 1; i < nMRTs; i++){
    currentColorRT[i] = TEXTURE_NONE;
    currentColorRTSlice[i] = NO_SLICE;
  }
  currentDepthRT = FB_DEPTH;
  currentDepthSlice = NO_SLICE;
  nCurrentRenderTargets = 1;


  currentStructureBuffer = -1;

  reset();
}

void Renderer::reset(const uint flags)
{
  //reset selected state
  if (flags & RESET_SHADER) selectedShader = SHADER_NONE;
  if (flags & RESET_VF) selectedVertexFormat = VF_NONE;

  if (flags & RESET_VB){
    for (uint i = 0; i < MAX_VERTEXSTREAM; i++){
      selectedVertexBuffers[i] = VB_NONE;
      selectedOffsets[i] = 0;
    }
  }
  if (flags & RESET_IB) selectedIndexBuffer = IB_NONE;

  if (flags & RESET_DS){
    selectedDepthState = DS_NONE;
    selectedStencilRef = 0;
  }
  if (flags & RESET_BS){
    selectedBlendState = BS_NONE;
    selectedSampleMask = ~0;
  }
  if (flags & RESET_RS) selectedRasterizerState = RS_NONE;

  if (flags&RESET_SB) selectedStructureBuffer = SB_NONE;
}

void Renderer::applyVertexStates()
{
  changeVertexFormat(selectedVertexFormat);
  for (uint i = 0; i < MAX_VERTEXSTREAM; i++){
    if (selectedVertexBuffers[i] != DONTCARE){
      changeVertexBuffer(i, selectedVertexBuffers[i], selectedOffsets[i]);
    }
  }
  if (selectedIndexBuffer != DONTCARE) changeIndexBuffer(selectedIndexBuffer);
}

void Renderer::apply(bool apply_shader_res){

  if (selectedShader != DONTCARE){
    changeShader(selectedShader);
	if (apply_shader_res)
		applyConstants();
  }

  /*	for (uint i = 0; i < nImageUnits; i++){
  if (selectedTextures[i] != DONTCARE) changeTexture(i, selectedTextures[i]);
  }*/
  if (apply_shader_res)
  {
	  applyTextures();

	  /*	for (uint i = 0; i < MAX_SAMPLERSTATE; i++){
	  if (selectedSamplerStates[i] != DONTCARE) changeSamplerState(i, selectedSamplerStates[i]);
	  }*/
	  applySamplerStates();
  }

  applyVertexStates();

  if (selectedDepthState != DONTCARE){
    changeDepthState(selectedDepthState, selectedStencilRef);
  }
  if (selectedBlendState != DONTCARE){
    changeBlendState(selectedBlendState, selectedSampleMask);
  }
  if (selectedRasterizerState != DONTCARE) changeRasterizerState(selectedRasterizerState);

  //	reset();
}

/*
void Renderer::clear(){
int index;

reset();
apply();

resetTextureUnits();

index = textures.getCount();
while (index--)	deleteTexture(index);

index = shaders.getCount();
while (index--)	deleteShader(index);
}
*/

TextureID Renderer::addTexture(const char *fileName, const bool useMipMaps, const SamplerStateID samplerState, uint flags){
  Image img;

  uint loadFlags = 0;
  if (!useMipMaps) loadFlags |= DONT_LOAD_MIPMAPS;

  if (img.loadImage(fileName, loadFlags)){
    if (img.getFormat() == FORMAT_RGBE8) img.unpackImage();
    if (useMipMaps && img.getMipMapCount() <= 1) img.createMipMaps();
    return addTexture(img, samplerState, flags);
  }
  else {
    char str[256];
    sprintf(str, "Couldn't open \"%s\"", fileName);

    g_logger->debug(WIP_ERROR, str);
    return TEXTURE_NONE;
  }
}

TextureID Renderer::addCubemap(const char **fileNames, const bool useMipMaps, const SamplerStateID samplerState, const int nArraySlices, uint flags){
  Image img;
  if (img.loadSlicedImage(fileNames, 0, nArraySlices)){
    if (img.getFormat() == FORMAT_RGBE8) img.unpackImage();
    if (useMipMaps && img.getMipMapCount() <= 1) img.createMipMaps();
    return addTexture(img, samplerState, flags);
  }
  else {
    char str[1024];
    int n = sprintf(str, "Couldn't open cubemap:\n");
    for (int i = 0; i < 6 * nArraySlices; i++){
      n += sprintf(str + n, "%s\n", fileNames[i]);
    }

    g_logger->debug(WIP_ERROR, str);

    return TEXTURE_NONE;
  }
}

TextureID Renderer::addNormalMap(const char *fileName, const FORMAT destFormat, const bool useMipMaps, const SamplerStateID samplerState, float sZ, float mipMapScaleZ, uint flags){
  Image img;

  uint loadFlags = 0;
  if (!useMipMaps) loadFlags |= DONT_LOAD_MIPMAPS;

  if (img.loadImage(fileName, loadFlags)){
    if (useMipMaps && img.getMipMapCount() <= 1) img.createMipMaps();
    if (img.toNormalMap(destFormat, sZ, mipMapScaleZ)){
      return addTexture(img, samplerState, flags);
    }
  }
  else {
    char str[256];
    sprintf(str, "Couldn't open \"%s\"", fileName);

    g_logger->debug(WIP_ERROR, str);

  }
  return TEXTURE_NONE;
}

ShaderID Renderer::addShader(const char *fileName, const uint flags){
  return addShader(fileName, NULL, 0, NULL, flags);
}

ShaderID Renderer::addShader(const char *fileName, const char *extra, const uint flags){
  return addShader(fileName, NULL, 0, extra, flags);
}

ShaderID Renderer::addShader(const char *fileName, const char **attributeNames, const int nAttributes, const char *extra, const uint flags){
  FILE *file = fopen(fileName, "rb");
  ShaderID res = SHADER_NONE;

  if (file == NULL){
    g_logger->debug(WIP_ERROR, "Couldn't load \"%s\"",fileName);
  }
  else {
#ifdef _DEBUG
    char str[66];
    str[0] = '\n';
    ::memset(str + 1, '-', sizeof(str) - 2);
    str[sizeof(str) - 1] = '\0';
    size_t lfn = strlen(fileName);
    size_t start = (sizeof(str) - lfn) / 2;

    str[start - 1] = '[';
    str[start + lfn] = ']';
    ::strncpy(str + start, fileName, lfn);
    g_logger->debug(WIP_INFO, str);
#endif

    // Find file size
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *shaderText = new char[length + 1];
    fread(shaderText, length, 1, file);
    fclose(file);
    shaderText[length] = '\0';

    char *vs = strstr(shaderText, "[Vertex shader]");
    char *gs = strstr(shaderText, "[Geometry shader]");
    char *fs = strstr(shaderText, "[Fragment shader]");

    char *header = (shaderText[0] != '[') ? shaderText : NULL;

    int vsLine = 0;
    if (vs != NULL){
      *vs = '\0';
      vs += 15;
      while (*vs == '\r' || *vs == '\n') vs++;

      char *str = shaderText;
      while (str < vs){
        if (*str == '\n') vsLine++;
        str++;
      }
    }

    int gsLine = 0;
    if (gs != NULL){
      *gs = '\0';
      gs += 17;
      while (*gs == '\r' || *gs == '\n') gs++;

      char *str = shaderText;
      while (str < gs){
        if (*str == '\n') gsLine++;
        str++;
      }
    }

    int fsLine = 0;
    if (fs != NULL){
      *fs = '\0';
      fs += 17;
      while (*fs == '\r' || *fs == '\n') fs++;

      char *str = shaderText;
      while (str < fs){
        if (*str == '\n') fsLine++;
        str++;
      }
    }

    res = addShader(vs, gs, fs, vsLine, gsLine, fsLine, header, extra, fileName, attributeNames, nAttributes, flags);
    delete[] shaderText;
  }
  return res;
}

int Renderer::getFormatSize(const AttributeFormat format) const {
  static int formatSize[] = { sizeof(float), sizeof(half), sizeof(u8) };
  return formatSize[format];
}

FontID Renderer::addFont(const char *textureFile, const char *fontFile, const SamplerStateID samplerState){
  FILE *file = fopen(fontFile, "rb");
  if (file == NULL) return FONT_NONE;

  TexFont font;
  uint version = 0;
  fread(&version, sizeof(version), 1, file);
  fread(font.chars, sizeof(font.chars), 1, file);
  fclose(file);

  if ((font.texture = addTexture(textureFile, false, samplerState)) == TEXTURE_NONE) return FONT_NONE;

  fonts.push_back(font);

  return fonts.size()-1;
}

void Renderer::setShaderConstant1i(const char *name, const int constant){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, &constant, sizeof(constant));
}

void Renderer::setShaderConstant1f(const char *name, const float constant){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, &constant, sizeof(constant));
}

void Renderer::setShaderConstant2f(const char *name, const RBVector2 &constant){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, &constant, sizeof(constant));
}

void Renderer::setShaderConstant3f(const char *name, const RBVector3 &constant){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, &constant, sizeof(constant));
}

void Renderer::setShaderConstant4f(const char *name, const RBVector4 &constant){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, &constant, sizeof(constant));
}

void Renderer::setShaderConstant4x4f(const char *name, const RBMatrix &constant){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, &constant, sizeof(constant));
}

void Renderer::setShaderConstantArray1f(const char *name, const float *constant, const uint count){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, constant, count * sizeof(float));
}

void Renderer::setShaderConstantArray2f(const char *name, const RBVector2 *constant, const uint count){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, constant, count * sizeof(RBVector2));
}

void Renderer::setShaderConstantArray3f(const char *name, const RBVector3 *constant, const uint count){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, constant, count * sizeof(RBVector3));
}

void Renderer::setShaderConstantArray4f(const char *name, const RBVector4 *constant, const uint count){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, constant, count * sizeof(RBVector4));
}

void Renderer::setShaderConstantArray4x4f(const char *name, const RBMatrix *constant, const uint count){
  CHECK(selectedShader != SHADER_NONE);
  setShaderConstantRaw(name, constant, count * sizeof(RBMatrix));
}

float Renderer::getTextWidth(const FontID font, const char *str, int length) const {
  if (font < 0) return 0;
  if (length < 0) length = (int)strlen(str);

  const Character *chars = fonts[font].chars;

  float len = 0;
  for (int i = 0; i < length; i++){
    len += chars[(unsigned char)str[i]].ratio;
  }

  return len;
}

uint Renderer::getTextQuads(const char *str) const {
  uint n = 0;
  while (*str){
    if (*str != '\n') n++;
    str++;
  }
  return n;
}

void Renderer::fillTextBuffer(TexVertex *dest, const char *str, float x, float y, const float charWidth, const float charHeight, const FontID font) const {
  float startx = x;

  while (*str){
    if (*str == '\n'){
      y += charHeight;
      x = startx;
    }
    else {
      Character chr = fonts[font].chars[*(unsigned char *)str];
      float cw = charWidth * chr.ratio;

      //drawQuad(x, y, x + cw, y + charHeight, NULL, chr.x0, chr.y0, chr.x1, chr.y1);
      dest[0].position = RBVector2(x, y);
      dest[0].texCoord = RBVector2(chr.x0, chr.y0);
      dest[1].position = RBVector2(x + cw, y);
      dest[1].texCoord = RBVector2(chr.x1, chr.y0);
      dest[2].position = RBVector2(x, y + charHeight);
      dest[2].texCoord = RBVector2(chr.x0, chr.y1);

      dest[3].position = RBVector2(x, y + charHeight);
      dest[3].texCoord = RBVector2(chr.x0, chr.y1);
      dest[4].position = RBVector2(x + cw, y);
      dest[4].texCoord = RBVector2(chr.x1, chr.y0);
      dest[5].position = RBVector2(x + cw, y + charHeight);
      dest[5].texCoord = RBVector2(chr.x1, chr.y1);

      dest += 6;
      x += cw;
    }
    str++;
  }
}

bool Renderer::drawText(const char *str, float x, float y, const float charWidth, const float charHeight, const FontID font, const SamplerStateID samplerState, const BlendStateID blendState, const DepthStateID depthState){
  if (font == FONT_NONE) return false;

  uint n = 6 * getTextQuads(str);
  if (n == 0) return true;

  if (n > fontBufferCount){
    fontBuffer = (TexVertex *)realloc(fontBuffer, n * sizeof(TexVertex));
    fontBufferCount = n;
  }

  fillTextBuffer(fontBuffer, str, x, y, charWidth, charHeight, font);

  drawTextured(PRIM_TRIANGLES, fontBuffer, n, fonts[font].texture, samplerState, blendState, depthState);

  return true;
}

void Renderer::setViewport(const int width, const int height){
  viewportWidth = width;
  viewportHeight = height;
}

void Renderer::resetStatistics(){
  nDrawCalls = 0;
}

#ifdef PROFILE
void Renderer::profileFrameStart(const float frameTime){
  profileStringIndex = sprintf(profileString, "Frametime: %.2fms\n\n", frameTime );
  profileFrame = true;

  profileTimeIndex = 0;
  if (profileReset){
    memset(profileTimes, 0, sizeof(profileTimes));
    profileFrameCount = 0;
  }

  profileFrameCount++;
}

void Renderer::profileFrameEnd(){
  profileReset = !profileFrame;
  profileFrame = false;
}

void Renderer::profileBegin(const char *name){
  if (profileFrame){
    finish();

    profileStartTime = g_base_app->get_time_in_millisecond();
    profileStringIndex += sprintf(profileString + profileStringIndex, "%s: ", name);
  }
}

void Renderer::profileNext(const char *name){
  if (profileFrame){
    finish();

	float currTime = g_base_app->get_time_in_millisecond();
    profileTimes[profileTimeIndex] += (currTime - profileStartTime);

    profileStringIndex += sprintf(profileString + profileStringIndex, "%.2fms\n%s: ", (profileTimes[profileTimeIndex] / profileFrameCount) , name);

    profileStartTime = currTime;
    profileTimeIndex++;
  }
}

void Renderer::profileEnd(){
  if (profileFrame){
    finish();
	float currTime = g_base_app->get_time_in_millisecond();
    profileTimes[profileTimeIndex] += (currTime - profileStartTime);

    profileStringIndex += sprintf(profileString + profileStringIndex, "%.2fms\n", (profileTimes[profileTimeIndex] / profileFrameCount) );

    profileTimeIndex++;
  }
}
#endif
