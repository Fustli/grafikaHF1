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

float Distance(vec2 pos1, vec2 pos2){
    return sqrt(pow(pos1.x - pos2.x , 2) + pow(pos2.y - pos2.y, 2));
}

vec2 DirVector(vec2 pos1, vec2 pos2){
    return vec2(pos2.x - pos1.x, pos2.y - pos1.y);
}

Camera2D camera;
GPUProgram gpuProgram; // vertex and fragment shaders

class Atom{
    vec2 pos, pivotPoint = 0, F = 0;
    vec3 color;
    float charge = 0;
    unsigned int vao, vbo;
    float phi;
    vec2 wTranslate;
    float mass, r;

    const int nv = 30;

public:
    Atom(vec2 pos, vec3 color, int charge, float mass){
        this->pos.x = pos.x;
        this->pos.y = pos.y;
        this->color.x = color.x;
        this->color.y = color.y;
        this->color.z = color.z;
        this->charge = charge * 1.602e-19;
        this->mass = mass;
    }

    float getR(){
        return r;
    }

    void setR(float R){
        this->r = R;
    }

    vec2 getForce(){
        return F;
    }

    void setForce(vec2 F){
        this->F = F;
    }

    vec2 getPos(){
        return pos;
    }

    void setPos(vec2 pos){
        this->pos.x = pos.x;
        this->pos.y = pos.y;
    }

    int getCharge(){
        return charge;
    }

    float getMass(){
        return mass;
    }

    void Animate(float dt, float omega, vec2 pivotPoint, vec2 velocity){
        phi = phi + omega + dt;
        pos = pos + velocity + dt;
        this->pivotPoint = pivotPoint;
    }

    mat4 M() {
		mat4 Mscale(0.6f, 0, 0, 0,
			0, 0.6f, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 1); // scaling

        mat4 MpivotTranslate(1, 0, 0, 0,
                             0, 1, 0, 0,
                             0, 0, 1, 0,
                             -pivotPoint.x, -pivotPoint.y, 0, 1);
        mat4 MinversePivotTranslate(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    -(-pivotPoint.x), -(-pivotPoint.y), 0, 1);

		mat4 Mtranslate(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
			pos.x, pos.y, 0, 1); // translation

		return Mscale * Mtranslate * MinversePivotTranslate * MpivotTranslate;	// model transformation
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
int posDif = 1;



class Molecules{
    std::vector<Atom> Atoms;
    Line line;
    vec2 centreOfMass, velocity = vec2(0,0), F = 0;
    int atomNum = 0;
    float omega = 0, mass = 0;
    const float hydrogenMass = 1.66e-24;
    float theta;
    vec3 M = vec3(0,0,0);


public:

    float getTheta(){
        return theta;
    }

    void setTheta(float theta){
        this->theta = theta;
    }

    float getOmega(){
        return omega;
    }

    void setOmega(float omega){
        this->omega = omega;
    }

    float getMass(){
        return mass;
    }

    void setMass(float m){
        this->mass = m;
    }

    vec2 getForce(){
        return F;
    }

    void setForce(vec2 F){
        this->F = F;
    }

    vec3 getM(){
        return M;
    }

    void setM(vec3 M){
        this->M = M;
    }

    vec2 getCentreOfMass(){
        return centreOfMass;
    }

    std::vector<Atom> getAtoms(){
        return Atoms;
    }

    vec2 getVelocity(){
        return velocity;
    }

    void setVelocity(vec2 velocity){
        this->velocity = velocity;
    }

    int getAtomSize(){
        return Atoms.size();
    }

    vec2 CalcCentreOfMass(){
        float mX = 0;
        float mY = 0;
        float M = 0;
        for (int i = 0; i <= atomNum-1; ++i) {
            mX += Atoms[i].getMass() * Atoms[i].getPos().x;
            mY += Atoms[i].getMass() * Atoms[i].getPos().y;
            M += Atoms[i].getMass();
        }
        mX /= M;
        mY /= M;
        return vec2(mX,mY);
    }

    float CalcTorque(){
        float tempTheta;

        //tömegközéppont eltolása az origóba, atomok vele tolása
        for (int i = 0; i <= atomNum-1; ++i) {
            Atoms[i].setPos(vec2(Atoms[i].getPos() - centreOfMass));
        }

        //tehetetlenségi nyomaték számolása
        for (int i = 0; i <= atomNum-1; ++i) {
            tempTheta += (Atoms[i].getMass()) * (dot(Atoms[i].getPos(),Atoms[i].getPos()));
        }

        //tömegközéppont eltolása az origóból, atomok vele tolása
        for (int i = 0; i <= atomNum-1; ++i) {
            Atoms[i].setPos(vec2(Atoms[i].getPos() + centreOfMass));
        }

        return tempTheta;
    }

    void Create() {
        line.Clear();
        Atoms.clear();

        atomNum = RandomNumber(2,8);
        int charge[atomNum];
        int mass[atomNum];
        int sumCharges = 0;
        for (int i = 0; i <= atomNum-2; ++i) {
            int tempCharge = 0;
            int tempMass = 0;
            while (tempCharge == 0){
                tempCharge = RandomNumber(-10,10);
                tempMass = rand() % 9 + 1;
            }
            charge[i] = tempCharge;
            mass[i] = tempMass;
            sumCharges += charge[i];
        }

        charge[atomNum-1] = sumCharges * -1;

        for (int i = 0; i <= atomNum-1; ++i) {
            float posX = RandomNumber(0, 1000) * posDif;
            float posY = RandomNumber(0, 1000) * posDif;

            if(charge[i] > 0) {
                Atom temp = Atom(vec2(posX / 150, posY / 150), vec3(1, 0, 0), charge[i], (float)mass[i] * hydrogenMass);
                Atoms.push_back(temp);
                temp.Create();
            }
            else{
                Atom temp = Atom(vec2(posX / 150, posY / 150), vec3(0, 0, 1), charge[i], (float)mass[i] * hydrogenMass);
                Atoms.push_back(temp);
                temp.Create();
            }
        }
        centreOfMass = CalcCentreOfMass();
        theta = CalcTorque();

        for (int i = 1; i <= atomNum-1; ++i) {
            line.AddLine(Atoms[i-1], Atoms[i]);
        }
        line.Create();
        posDif *= -1;
    }

    void Draw() {
        line.Draw();
        for (int i = 0; i <= atomNum-1; ++i) {
            Atoms[i].Draw();
        }
    }

    void Animate(float dt){
        for (int i = 0; i <= Atoms.size()-1; ++i) {
            Atoms[i].Animate(dt, omega, centreOfMass, velocity);
        }
    }
};







std::vector<Molecules> molecules;
Molecules molecule1;
Molecules molecule2;

void Move(float dt){
    for (int i = 0; i <= molecules.size()-1; ++i) {
        for (int j = 0; j <= molecules[i].getAtomSize()-1; ++j) {
            for (int k = 0; k <= molecules.size()-1; ++k) {
                if(i != k){
                    for (int l = 0; l <= molecules[l].getAtomSize()-1; ++l) {
                        float tempF = 0;
                        tempF = molecules[i].getAtoms()[j].getCharge() + (molecules[i].getAtoms()[j].getCharge() +
                                molecules[k].getAtoms()[l].getCharge())
                                / (2 * M_PI * 8.85e-3 * length(molecules[i].getAtoms()[j].getPos() - molecules[k].getAtoms()[l].getPos()
                                * (molecules[k].getAtoms()[l].getPos() - molecules[i].getAtoms()[j].getPos())));
                        molecules[i].getAtoms()[j].setForce(tempF);
                    }
                }
            }
            molecules[i].getAtoms()[j].setForce(molecules[i].getAtoms()[j].getForce() - 1 * molecules[i].getVelocity());

            vec2 r = DirVector(molecules[i].getCentreOfMass(), molecules[i].getAtoms()[j].getPos());
            molecules[i].setM(molecules[i].getM() + cross(r, molecules[i].getAtoms()[j].getForce()));
            molecules[i].setForce(molecules[i].getForce() + (molecules[i].getAtoms()[j].getForce() * normalize(r)));
        }

        molecules[i].setVelocity(molecules[i].getVelocity() + (molecules[i].getForce() / molecules[i].getMass()) * dt);
        molecules[i].setOmega(molecules[i].getOmega() + (molecules[i].getM().z) / molecules[i].CalcTorque() * dt);
        molecules[i].Animate(dt);
    }


}


void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    glLineWidth(1.0f);
    srand(time(NULL));

    //molecule.Create();
    molecules.push_back(molecule1);
    molecules.push_back(molecule2);



    gpuProgram.create(vertexSource, fragmentSource, "outColor");

}

void onDisplay() {
    glClearColor(0.5f, 0.5f, 0.5f, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i <= molecules.size()-1; ++i) {
        molecules[i].Draw();
    }

    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
    switch (key) {
        case 'd': camera.Pan(vec2(-0.1f, 0)); break;
        case 'a': camera.Pan(vec2(+0.1f, 0)); break;
        case 's': camera.Pan(vec2(0, 0.1f)); break;
        case 'w': camera.Pan(vec2(0, -0.1f)); break;
        case ' ': molecules.clear();
                  molecule1.Create();
                  molecule2.Create();
                  molecules.push_back(molecule1);
                  molecules.push_back(molecule2);
                  break;
    }
    glutPostRedisplay();
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {

}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {
	}


float dt = 0.01;
void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program

    /*if(time % 10 == 0){
        Move(dt);
    }*/

    glutPostRedisplay();					// redraw the scene
}
