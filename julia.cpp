#include <exception>
#include <functional>
#include <SDL2/SDL.h>
#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>

// Shader sources
const GLchar* vertexSource =
    "attribute vec4 position;    \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = vec4(position.xyz, 1.0);  \n"
    "}                            \n";
const GLchar* fragmentSource =
"precision mediump float;\n"
"uniform vec2 C;\n"
//osc is Offset xy and scale z
"uniform vec3 osc;\n"
"vec2 Z, dZ, sqr_Z;\n"
"float sqr_norm_Z, sqr_norm_dZ;\n"
"void main() {\n"
"int max_iter_count = 32;\n"
//"vec3 osc=vec3(0.0,0.0,1.0);\n"
//"Z = vec2(7.111 * ((gl_FragCoord.x / 1920.0)-0.5),4.0 * ((gl_FragCoord.y / 1080.0)-0.5));\n"
"Z = vec2(7.111 * osc.z * ((gl_FragCoord.x / 1920.0) + osc.x -0.5),\n"
	"4.0 * osc.z * ((gl_FragCoord.y / 1080.0) + osc.x -0.5));\n"
"dZ = vec2(1., 0.);\n"
"for(int i = 0; i < 20; i++) {\n"
"sqr_Z = vec2(Z[0] * Z[0], Z[1] * Z[1]);\n"
"if (sqr_Z[0] + sqr_Z[0] > 100.0) {\n"
    "break;}\n"
    "dZ = vec2(\n"
     " 2. * (Z.x * dZ.x - Z.y * dZ.y) + 1.,\n"
     " 2. * (Z.x * dZ.y + Z.y * dZ.x));\n"
     " Z = vec2(Z.x * Z.x - Z.y * Z.y, 2. * Z.x * Z.y) + C;\n"
"}\n"
  "sqr_norm_Z = Z.x * Z.x + Z.y * Z.y;\n"
  "sqr_norm_dZ = dZ.x * dZ.x + dZ.y * dZ.y;\n"
  "vec4 color0 = vec4(0.0, 0.0, 0.4, 1.0);\n"
  "vec4 color1 = vec4(0.6, 1.0, 1.0, 1.0);\n"
  "gl_FragColor = mix(color0,color1,sqrt(sqrt(sqr_norm_Z / sqr_norm_dZ) * .5 * log(sqr_norm_Z)));}";




std::function<void()> loop;
void main_loop() { loop(); }

int main(int argc, char** argv)
{
    // SDL_Init(SDL_INIT_VIDEO);

    auto wnd(
        SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    auto glc = SDL_GL_CreateContext(wnd);

    auto rdr = SDL_CreateRenderer(
        wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    // Create Vertex Array Object
   // GLuint vao;
   // glGenVertexArrays(1, &vao);
   // glBindVertexArray(vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f};
//    GLfloat vertices[] = {-0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f,-0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);


    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLfloat C[2]={0.0, 0.0};
    GLfloat osc[3]={0.0, 0.0, 1.0};
    //GLfloat C=0.0;
    int mousex,mousey,winx,winy;

    SDL_GetWindowSize(wnd,&winy,&winx);
    GLuint pC=glGetUniformLocation(shaderProgram, "C");
    GLuint posc=glGetUniformLocation(shaderProgram, "osc");
	printf("%i\n",posc);
    GLfloat colorise=0.5;
    glUniform2fv(pC,1,C);
    glUniform3fv(posc,1,osc);


    loop = [&]
    {
//    	colorise+=0.01;
	if(colorise>1.0)colorise=0;
	SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            switch(e.type){
            case SDL_QUIT:
            case SDL_KEYDOWN:
            	std::terminate();
            	break;
            case SDL_MOUSEMOTION:
            	SDL_GetMouseState(&mousey, &mousex);
            	colorise=(mousex/(float)winx);
            	C[1]=6.0*(((float)mousey/(float)winy)-0.5);
            	C[0]=6.0*(((float)mousex/(float)winx)-0.5);
	        glUniform2fv(pC,1,C);
		break;
	    case SDL_MOUSEWHEEL:
		if(e.wheel.y > 0) {
			if(osc[2]< 2.0) osc[2]+=0.1;
			glUniform3fv(posc,1,osc);
		} else {
			if(osc[2] > 0.1) osc[2]-=0.1;
			glUniform3fv(posc,1,osc);
		}
		break;

            }


        }


        // Clear the screen to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Draw a triangle from the 3 vertices
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(wnd);
    };
    while(true) main_loop();
    return 0;
}
