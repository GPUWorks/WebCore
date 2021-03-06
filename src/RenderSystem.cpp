#include "RenderSystem.h"
#include "shaders.h"
#include "Mesh.h"
#include <iostream>


RenderSystem::RenderSystem()
{
}


void RenderSystem::init(WindowManager& window)
{
  // Assign vars
  m_window = window;

  // GL Setup
  glEnable(GL_DEPTH_TEST);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable( GL_BLEND );

   // Geometry pass shaders

	const char vertexShaderSource[] =
		"attribute vec4 vPosition;		                     \n"
    "attribute vec3 vNormal;		                     \n"
    "attribute vec2 vUV; \n"
    "uniform mat4 ModelLocal; \n"
    "uniform mat4 ViewProj; \n"
    "uniform vec3 diffuse;  \n"
		"varying vec3 color;                                 \n" // To FS
    "varying vec2 uv;                                 \n"
		"void main()                                         \n"
		"{                                                   \n"
		"   gl_Position = ViewProj * (ModelLocal * vPosition);                         \n"
    "   color = diffuse;             \n"
    "   uv.x = vUV.x; \n"
    "   uv.y = vUV.y; \n"
		"}                                                   \n";

	const char fragmentShaderSource[] =
		"precision mediump float;                     \n"
    "uniform sampler2D BaseMap;									\n"
    "uniform float alpha;									\n"
		"varying vec3 color;                          \n"   // From VS
    "varying vec2 uv;                                 \n"
		"void main()                                  \n"
		"{                                            \n"
    "  vec3 shade = texture2D(BaseMap, uv).rgb; \n"
    "  gl_FragColor = vec4(color * shade, 1.0);"
		"}                                            \n";


    const char vertexShaderSource2[] =
      "attribute vec4 vPosition;		                     \n"
      "uniform mat4 ModelLocal; \n"
      "uniform mat4 ViewProj; \n"
      "uniform vec3 diffuse;  \n"
      "varying vec3 color;                                 \n" // To FS
      "void main()                                         \n"
      "{                                                   \n"
      "   gl_Position = ViewProj * (ModelLocal * vPosition);                         \n"
      "   color = diffuse;             \n"
      "}                                                   \n";

    const char fragmentShaderSource2[] =
      "precision mediump float;                     \n"
      "uniform float alpha;									\n"
      "varying vec3 color;                          \n"   // From VS
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4(color, 1.0);"
      "}                                            \n";

	//load vertex and fragment shaders
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	programObject = buildProgram(vertexShader, fragmentShader, "vPosition", "Vuv");

  GLuint vertexShader2 = loadShader(GL_VERTEX_SHADER, vertexShaderSource2);
  GLuint fragmentShader2 = loadShader(GL_FRAGMENT_SHADER, fragmentShaderSource2);
  programObject2 = buildProgram(vertexShader2, fragmentShader2, "vPosition", 0);

  attribute_v_coord = glGetAttribLocation(programObject, "vPosition");
  attribute_v_coord2 = glGetAttribLocation(programObject2, "vPosition");
  attribute_v_uv = glGetAttribLocation(programObject, "vUV");

  // Tex
  GLint texLoc;
  texLoc = glGetUniformLocation(programObject, "BaseMap");
  glUniform1i(texLoc, 0);

	//save location of uniform variables
  uniformDiffuse = glGetUniformLocation(programObject, "diffuse");
  uniformDiffuse2 = glGetUniformLocation(programObject2, "diffuse");
  uniformVP = glGetUniformLocation(programObject, "ViewProj");
  uniformML = glGetUniformLocation(programObject, "ModelLocal");
  uniformVP2 = glGetUniformLocation(programObject2, "ViewProj");
  uniformML2 = glGetUniformLocation(programObject2, "ModelLocal");
  uniformAlpha = glGetUniformLocation(programObject, "alpha");
}


RenderSystem::~RenderSystem()
{


}


void RenderSystem::zoom(int i)
{
  if (i > 0) m_zoomFactor += 1;
  if (i < 0) m_zoomFactor -= 1;
  if (m_zoomFactor < 2) m_zoomFactor = 2;
  if (m_zoomFactor > 6) m_zoomFactor = 6;

  m_zoom = 0.5 + 0.5 * (1<<m_zoomFactor);
}


void RenderSystem::move(double x, double y)
{
   m_cameraX += 0.05 * -x / m_zoom + 0.05 * y / m_zoom;
   m_cameraY += 0.05 *  x / m_zoom + 0.05 * y / m_zoom;
}


void RenderSystem::start()
{
  glClearColor(0.f, 92.f / 255.f, 159.f / 255.f, 1.f); // "Skydiver"
  glClear( GL_COLOR_BUFFER_BIT );

  glViewport(0, 0, m_window.width, m_window.height);

  glDepthFunc(GL_LESS);

  // Enable our shader programlightmap_house1
  //glUseProgram(programObject);

  // Camera view

  float v_side = 0.05f / m_zoom;
  glm::mat4 Projection = glm::ortho(
    -v_side * m_window.width,
     v_side * m_window.width,
    -v_side * m_window.height,
     v_side * m_window.height,
     1.0f,
     100.0f);

  // Camera matrix
  glm::mat4 View = glm::lookAt(
                glm::vec3(m_cameraX+30, m_cameraY+30, 40), // Camera is at (4,3,3), in World Space
                glm::vec3(m_cameraX+0,  m_cameraY+0, 0), // and looks at the origin
                glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
               );

  glm::mat4 VP = Projection * View;

  glUseProgram(programObject);
  glUniformMatrix4fv(uniformVP, 1, GL_FALSE, &VP[0][0]);
  glUseProgram(programObject2);
  glUniformMatrix4fv(uniformVP2, 1, GL_FALSE, &VP[0][0]);


  // DEBUG

  //glm::mat4 local = glm::rotate(VP, 3.14f, glm::vec3(0.f, 0.f, 1.f));
  //glUniformMatrix4fv(uniformVP, 1, GL_FALSE, &local[0][0]);
  // glm::mat4 local = glm::mat4(1.0f);
  // glUniformMatrix4fv(uniformML, 1, GL_FALSE, &local[0][0]);
  //
  // float colour0[] = { 123.f / 255.f, 179.f / 255.f,  105.f / 255.f }; // Grass Green
  // float colour1[] = { 188.f / 255.f, 188.f / 255.f,  100.f / 255.f }; // Yellow
  //
  // glUniform3fv(uniformDiffuse, 1, colour0);
  //
  // drawObjModel_square();
}


void RenderSystem::end()
{
  glUseProgram(programObject);
}


void RenderSystem::setCursor(int x, int y)
{
  float v_side = 0.05f / m_zoom;
  glm::mat4 Projection = glm::ortho(
    -v_side * m_window.width,
     v_side * m_window.width,
    -v_side * m_window.height,
     v_side * m_window.height,
     1.0f,
     100.0f);

  glm::mat4 View = glm::lookAt(
                glm::vec3(m_cameraX+30, m_cameraY+30, 40), // Camera is at (4,3,3), in World Space
                glm::vec3(m_cameraX+0,  m_cameraY+0, 0), // and looks at the origin
                glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
               );

  glm::vec3 m_start;
  glm::vec3 m_end;
  glm::ivec4 viewport(0, 0, m_window.width, m_window.height);

  // window pos of mouse, Y is inverted on Windows
  glm::vec3 win((double)x, (double)(m_window.height - y), 0.0);

  // get point on the 'near' plane (third param is set to 0.0)
  m_start = glm::unProject(win, View, Projection, viewport);

  // get point on the 'far' plane (third param is set to 1.0)
  win.z = 1.0;
  m_end = glm::unProject(win, View, Projection, viewport);

  // now you can create a ray from m_start to m_end

  // Calculate some intersection point

  glm::vec3 m_dir = m_end - m_start;
  //(m_start + a * m_dir).z = 0;
  float a = -m_start.z / m_dir.z;
  m_cursorX = m_start.x + a * m_dir.x;
  m_cursorY = m_start.y + a * m_dir.y;
}


bool RenderSystem::enableVertexPos(GLint program)
{
  if (program == programObject)
  {
      glEnableVertexAttribArray(attribute_v_coord);
      att = attribute_v_coord;
      return true;
  }

  if (program == programObject2)
  {
      glEnableVertexAttribArray(attribute_v_coord2);
      att = attribute_v_coord2;
      return true;
  }

  att = 0;
  return false;
}


bool RenderSystem::enableVertexUV(GLint program)
{
  if (program == programObject)
  {
      glEnableVertexAttribArray(attribute_v_uv);
      att = attribute_v_uv;
      return true;
  }

  att = 0;
  return false;
}
