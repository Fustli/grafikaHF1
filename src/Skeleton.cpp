//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : 
// Neptun : 
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";
GPUProgram gpuProgram; // vertex and fragment shaders
unsigned int vertexArray;
unsigned int buffer;
unsigned int vbo;		// vertex buffer object

//"atomhoz" körhöz tartozó struct és tagváltozók
const int nv = 100;
const float PI = 3.1415;
const float angle = PI * 2 / nv;
float radius = 1.f;
const int r = 0.5;

struct Atom{
    vec2 pos;
    vec3 color;

    Atom(vec2 pos, float r, float g, float b){
        pos.x = this->pos.x;
        pos.y = this->pos.y;
        color.x = r;
        color.y = g;
        color.z = b;
    }

    void Create() {
        glGenVertexArrays(1, &buffer);	// get 1 vao id
        glBindVertexArray(buffer);		// make it active

        glGenBuffers(1, &vbo);	// Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // Geometry with 24 bytes (6 floats or 3 x 2 coordinates)

        vec2 vertices[nv];
        for (int i = 0; i < nv; i++) {
            float fi = i * 2 * M_PI / nv;
            vertices[i] = vec2(cosf(fi), sinf(fi));
        }
        glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
                     sizeof(vec2) * nv,  // # bytes
                     vertices,	      	// address
                     GL_STATIC_DRAW);	// we do not change later

        glEnableVertexAttribArray(0);  // AttribArray 0
        glVertexAttribPointer(0,       // vbo -> AttribArray 0
                              2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
                              0, NULL); 		     // stride, offset: tightly packed
    }

    void Draw(){
        for (int i = -4; i <= 4; i++){
            // Set color to (0, 1, 0) = green
            int location = glGetUniformLocation(gpuProgram.getId(), "color");

            float MVPtransf[4][4] = { 0.1f, 0, 0, 0,    // MVP matrix,
                                      0, 0.1f, 0, 0,    // row-major!
                                      0, 0, 0, 0,
                                      0.2f*i, 0.2f*i, 0, 1 };

            glUniform3f(location, 1, 0, 0); // 3 floats
            location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
            glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

            glBindVertexArray(buffer);  // Draw call
            glDrawArrays(GL_TRIANGLE_FAN, 0 /*startIdx*/, nv /*# Elements*/);
        }
    }
};

Atom Atom = Atom;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    Atom.Create();

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}


void onDisplay() {

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    Atom.Draw();


    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space

}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	}


// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}
