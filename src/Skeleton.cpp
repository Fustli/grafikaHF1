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
		vec4 temp = vec4(vp.x, vp.y, 0, 1) * MVP;
        float z = (temp.x * temp.x) + (temp.y * temp.y) + 1;
        float w = sqrt(z);
    gl_Position = vec4(temp.x/(w+1), temp.y/(w+1), 0, 1);		// transform vp from modeling space to normalized device space
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

// 2D camera
class Camera2D {
    vec2 wCenter; // center in world coordinates
    vec2 wSize;   // width and height in world coordinates
public:
    Camera2D() : wCenter(0, 0), wSize(10, 10) { }

    mat4 V() { return TranslateMatrix(-wCenter); }
    mat4 P() { return ScaleMatrix(vec2(2 / wSize.x, 2 / wSize.y)); }

    mat4 Vinv() { return TranslateMatrix(wCenter); }
    mat4 Pinv() { return ScaleMatrix(vec2(wSize.x / 2, wSize.y / 2)); }

    void Zoom(float s) { wSize = wSize * s; }
    void Pan(vec2 t) { wCenter = wCenter + t; }
};

float RandomNumber(float Min, float Max)
{
    return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}


Camera2D camera;
GPUProgram gpuProgram; // vertex and fragment shaders

class Atom{
    vec2 pos;
    vec3 color;
    int charge;
    unsigned int vao, vbo;
    float sx, sy, phi;
    vec2 wTranslate;
    float hydrogenMass = 1.66e-24;

    const int nv = 30;

public:
    Atom(vec2 pos, vec3 color, int charge){
        this->pos.x = pos.x;
        this->pos.y = pos.y;
        this->color.x = color.x;
        this->color.y = color.y;
        this->color.z = color.z;
        this->charge = charge;
    }

    vec2 getPos(){
        return pos;
    }

    mat4 M() {
		mat4 Mscale(0.3f, 0, 0, 0,
			0, 0.3f, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 1); // scaling

		mat4 Mtranslate(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
			pos.x, pos.y, 0, 1); // translation

		return Mscale * Mtranslate;	// model transformation
	}

    void Create() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);	// Generate 2 buffers
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

        glVertexAttribPointer(0,
                              2, GL_FLOAT, GL_FALSE,
                              0, NULL);



    }

    void Draw(){
            int location = glGetUniformLocation(gpuProgram.getId(), "color");
            glUniform3f(location, color.x, color.y, color.z);
            mat4 MVPTransform = M() * camera.V() * camera.P();
            gpuProgram.setUniform(MVPTransform, "MVP");

            glBindVertexArray(vao);  // Draw call
            glDrawArrays(GL_TRIANGLE_FAN, 0 /*startIdx*/, nv /*# Elements*/);
    }
};

class Line{
    unsigned int vao, vbo;
    std::vector<float> vertices;
    //std::vector<vec3>color;
    vec2 wTranslate;
    const int nl = 100;

public:
    void Create(){
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo); // Generate 1 vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glEnableVertexAttribArray(0);  // attribute array 0

        glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
                     sizeof(float) * vertices.size(),  // # bytes
                     &vertices[0],	      	// address
                     GL_STATIC_DRAW);	// we do not change later

        glVertexAttribPointer(0,
                              2, GL_FLOAT, GL_FALSE,
                              0, NULL);

    }

    void Clear(){
        vertices.clear();
    }

    void AddLine(Atom atom1, Atom atom2){
        vertices.push_back(atom1.getPos().x);
        vertices.push_back(atom1.getPos().y);
        vertices.push_back(atom2.getPos().x);
        vertices.push_back(atom2.getPos().y);
    }


    mat4 M() { // modeling transform
        return mat4(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    wTranslate.x, wTranslate.y, 0, 1); // translation
    }

    mat4 Minv() { // inverse modeling transform
        return mat4(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    -wTranslate.x, -wTranslate.y, 0, 1); // inverse translation
    }

    void Draw(){
       int location = glGetUniformLocation(gpuProgram.getId(), "color");
        glUniform3f(location, 1, 1, 1);

        mat4 MVPTransform = M() * camera.V() * camera.P();
        gpuProgram.setUniform(MVPTransform, "MVP");
        glBindVertexArray(vao);  // Draw call
        glDrawArrays(GL_LINES, 0 /*startIdx*/, nl /*# Elements*/);
    }
};

class Molecules{
    Line line;
    std::vector<Atom> Atoms1;
    std::vector<Atom> Atoms2;
    int atomNum1 = 0;
    int atomNum2 = 0;
    int randomNum = 0;
    float hydrogenMass = 1.66e-24;

public:

    void Create() {
        line.Clear();
        Atoms1.clear();
        Atoms2.clear();
        atomNum1 = RandomNumber(2,8);
        int charge1[atomNum1];
        int sumCharges1 = 0;
        for (int i = 0; i <= atomNum1-2; ++i) {
            int tempCharge = 0;
            while (tempCharge == 0){
                tempCharge = RandomNumber(-10,10);
            }
            charge1[i] = tempCharge;
            sumCharges1 += charge1[i];
        }

        charge1[atomNum1-1] = sumCharges1 * -1;

        for (int i = 0; i <= atomNum1-1; ++i) {
            float posX = RandomNumber(0, 700);
            float posY = RandomNumber(0, 700);

            if(charge1[i] > 0) {
                Atom temp = Atom(vec2(posX / 150, posY / 150), vec3(1, 0, 0), charge1[i]);
                Atoms1.push_back(temp);
                temp.Create();
            }
            else{
                Atom temp = Atom(vec2(posX / 150, posY / 150), vec3(0, 0, 1), charge1[i]);
                Atoms1.push_back(temp);
                temp.Create();
            }

        }

        atomNum2 = RandomNumber(2,8);
        int charge2[atomNum2];
        int sumCharges2 = 0;
        for (int i = 0; i <= atomNum2-2; ++i) {
            int tempCharge = 0;
            while (tempCharge == 0){
                tempCharge = RandomNumber(-10,10);
            }
            charge2[i] = tempCharge;
            sumCharges2 += charge2[i];
        }

        charge2[atomNum2-1] = sumCharges2 * -1;

        for (int i = 0; i <= atomNum2-1; ++i) {
            float posX, posY;
            posX = RandomNumber(-700, -100);
            posY = RandomNumber(-700, 600);

            if(charge2[i] > 0) {
                Atom temp = Atom(vec2(posX / 150, posY / 150), vec3(1, 0, 0), charge2[i]);
                Atoms2.push_back(temp);
                temp.Create();
            }
            else{
                Atom temp = Atom(vec2(posX / 150, posY / 150), vec3(0, 0, 1), charge2[i]);
                Atoms2.push_back(temp);
                temp.Create();
            }

        }

        for (int i = 1; i <= atomNum1-1; ++i) {
            line.AddLine(Atoms1[i-1], Atoms1[i]);
        }

        for (int i = 1; i <= atomNum2-1; ++i) {
            line.AddLine(Atoms2[i-1], Atoms2[i]);
        }

        line.Create();


    }

    void Draw() {
        for (int i = 0; i <= atomNum1-1; ++i) {
            Atoms1[i].Draw();
        }

        for (int i = 0; i <= atomNum2-1; ++i) {
            Atoms2[i].Draw();
        }

        line.Draw();
    }
};

Molecules molecule;


void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    glLineWidth(1.0f);
    srand(time(NULL)^getpid());

    //molecule.Create();


    gpuProgram.create(vertexSource, fragmentSource, "outColor");

}

void onDisplay() {
    glClearColor(0.5f, 0.5f, 0.5f, 0);
    glClear(GL_COLOR_BUFFER_BIT);


    molecule.Draw();


    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
    switch (key) {
        case 'd': camera.Pan(vec2(-0.1f, 0)); break;
        case 'a': camera.Pan(vec2(+0.1f, 0)); break;
        case 's': camera.Pan(vec2(0, 0.1f)); break;
        case 'w': camera.Pan(vec2(0, -0.1f)); break;
        case ' ': molecule.Create(); break;
    }
    glutPostRedisplay();
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
    float sec = time / 1000.0f;				// convert msec to sec
    //atom1.Animate(sec);
    glutPostRedisplay();					// redraw the scene
}
