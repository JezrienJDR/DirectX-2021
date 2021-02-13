#pragma once
#include <iostream>
#include <vector>

using namespace std;


// My vertex class
class vec3
{
public:
    float x;
    float y;
    float z;

    vec3(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    vec3()
        : x(0.0f), y(0.0f), z(0.0f)
    {

    }

    // Cross multiplication
    vec3 cross(vec3 v)
    {
        /*
         float i = (x * v.x) + (x * v.y) + (x * v.z);
         float j = (y * v.x) + (y * v.y) + (y * v.z);
         float k = (z * v.x) + (z * v.y) + (z * v.z);
        */

        float i = (y * v.z) - (z * v.y);
        float j = (z * v.x) - (x * v.z);
        float k = (x * v.y) - (y * v.x);


        vec3 n(i, j, k);

        return n;
    }
    
    vec3 add(vec3 b)
    {
        vec3 sum = vec3(x + b.x, y + b.y, z + b.z);
        return sum;
    }

    // returns the vector from self to the argument
    vec3 to(vec3 final)
    {
        float i = final.x - x;
        float j = final.y - y;
        float k = final.z - z;

        return vec3(i, j, k);
    }


    // returns the dot product of self and argument
    float dot(vec3 v)
    {
        float dotProduct = (
            (x * v.x) +
            (y * v.y) +
            (z * v.z));

        if (abs(dotProduct) < 0.001f)
        {
            dotProduct = 0;
        }

        return dotProduct;
    }

    // returns the distance from self to argument
    float length(vec3 o)
    {
        float dx = x - o.x;
        float dy = y - o.y;
        float dz = z - o.z;

        float squaredDistance = (dx * dx) + (dy * dy) + (dz * dz);
        float length = sqrt(squaredDistance);

        return length;
    }

    // returns a copy of self, rotated around Y axis
    vec3 Yrotate(float theta)
    {
        /*
            cos  0   sin
            0    1   0
            -sin 0  cos
        */

        float a = (theta * 3.141592654f) / 180.0f;

       
        vec3 col1 = vec3(cos(a), 0.0f, -sin(a));
        vec3 col2 = vec3(0.0f, 1.0f, 0.0f);
        vec3 col3 = vec3(sin(a), 0.0f, cos(a));


        float i = dot(col1);
        float j = dot(col2);
        float k = dot(col3);

        return vec3(i, j, k);

    }

    // returns a copy of self, rotated around X axis    
    vec3 Xrotate(float theta)
    {
        /*
            1  0   0
            0  cos -sin
            0  sin  cos
        */

        float a = (theta * 3.141592654f) / 180.0f;



        vec3 col1 = vec3(1.0f, 0.0f, 0.0f);
        vec3 col2 = vec3(0.0f, cos(a), sin(a));
        vec3 col3 = vec3(0.0f, -sin(a), cos(a));


        float i = dot(col1);
        float j = dot(col2);
        float k = dot(col3);

        return vec3(i, j, k);

    }


    // returns a copy of self, rotated around Z axis
    vec3 Zrotate(float theta)
    {
        /*
            cos  -sin 0
            sin  cos  0
            0    0    1
        */

        float a = (theta * 3.141592654f) / 180.0f;


        vec3 col1 = vec3(cos(a), sin(a), 0.0f);
        vec3 col2 = vec3(-sin(a), cos(a), 0.0f);
        vec3 col3 = vec3(0.0f, 0.0f, 1.0f);


        float i = dot(col1);
        float j = dot(col2);
        float k = dot(col3);

        return vec3(i, j, k);

    }



};


// Vertex class, containing more data
class vert
{
public:
    vert()
        : position(vec3(0.0f, 0.0f, 0.0f)), index(0.0f), u(0.0f), v(0.0f)
    {}

    vert(vec3 vec)
        : position(vec), index(0.0f), u(0.0f), v(0.0f)
    {

    }

    vec3 position;
    vec3 normal;
    int index;

    float u;
    float v;
};


// Triangle class 
class tri
{
public:

    // consists of 3 vertices
    vert a;
    vert b;
    vert c;


    tri()
    {    }

    tri(vert _a, vert _b, vert _c)
        : a(_a), b(_b), c(_c)
    {
        // constructor calculates the normals and assigns them to each vertex.

        vec3 vecB = a.position.to(b.position);
        vec3 vecC = a.position.to(c.position);

        normal = vecB.cross(vecC);

        a.normal = normal;
        b.normal = normal;
        c.normal = normal;
        
        a.u = 0.0f;
        a.v = 0.0f;

        b.u = 1.0f;
        b.v = 0.0f;

        c.u = 0.0f;
        b.u = 1.0f;
    }

    // alternate constructor using raw vectors instead of vertices.

    tri(vec3 _a, vec3 _b, vec3 _c)
        : a(vert(_a)), b(vert(_b)), c(vert(_c))
    {
        vec3 vecB = a.position.to(b.position);
        vec3 vecC = a.position.to(c.position);

        normal = vecB.cross(vecC);


        a.normal = normal;
        b.normal = normal;
        c.normal = normal;

        a.u = 0.0f;
        a.v = 0.0f;

        b.u = 1.0f;
        b.v = 0.0f;

        c.u = 0.0f;
        b.u = 1.0f;
    }

    vec3 normal;
};



// Quad class consisting of two triangles.
class quad
{
public:
    tri a;
    tri b;

    quad()
    {
        this->a = tri();
        this->b = tri();
    }

    // Takes in four vertices (or vectors) and generates two triangles between them.

    quad(vert a, vert b, vert c, vert d)
    {
        this->a = tri(a, c, b);
        this->b = tri(c, a, d);


 
    }

    quad(vec3 a, vec3 b, vec3 c, vec3 d)
    {
        this->a = tri(a, c, b);
        this->b = tri(c, a, d);
    }

    void UVreset()
    {
        a.a.u = 0;
        a.a.v = 0;
        
        a.b.u = 1;
        a.b.v = 1;

        a.c.u = 1;
        a.c.v = 0;

        b.a.u = 1;
        b.a.v = 1;

        b.b.u = 0;
        b.b.v = 0;

        b.c.u = 0;
        b.c.v = 1;
        
    
    }
};


// Circle class generates a ring of vec3s.
class Circle
{
public:

    vector<vec3> points;
    float radius;
    int subd;

    Circle(float _r = 1.0f, int _subd = 3)
    {
        radius = _r;
        if (radius <= 0)
        {
            radius = 1.0f;
        }

        subd = _subd;
        if (subd < 3)
        {
            subd = 3;
        }

        vec3 p1 = vec3(-radius, 0.0f, 0.0f);

        //float angle = ((360.0f / subd) * 3.141592654f / 180.0f);
        float angle = (360.0f / subd);
        points.push_back(p1);

        int count = 1;
        while (count < subd)
        {
            points.push_back(p1.Yrotate(angle * count));

            count++;
        }
    }


    Circle xRotate(float angle)
    {
        Circle temp = Circle(radius, subd);

        for (int i = 0; i < subd; i++)
        {
            temp.points[i] = points[i].Xrotate(angle);
        }

        return temp;
    }


    Circle yRotate(float angle)
    {
        Circle temp = Circle(radius, subd);

        for (int i = 0; i < subd; i++)
        {
            temp.points[i] = points[i].Yrotate(angle);
        }


        return temp;
    }


    Circle zRotate(float angle)
    {
        Circle temp = Circle(radius, subd);

        for (int i = 0; i < subd; i++)
        {
            temp.points[i] = points[i].Zrotate(angle);
        }


        return temp;
    }

    Circle copy()
    {
        Circle temp(radius, subd);

        for (int i = 0; i < subd; i++)
        {
            temp.points[i] = points[i];
        }

        return temp;

    }

    void translate(vec3 shift)
    {

        for (int i = 0; i < subd; i++)
        {
            points[i].x += shift.x;
            points[i].y += shift.y;
            points[i].z += shift.z;
        }
    }

    void Collapse()
    {
        float sumX = 0.0f;
        float sumY = 0.0f;
        float sumZ = 0.0f;

        for (vec3 p : points)
        {
            sumX += p.x;
            sumY += p.y;
            sumZ += p.z;
        }

        sumX = sumX / subd;
        sumY = sumY / subd;
        sumZ = sumZ / subd;
        
        /*
        for (vec3 p : points)
        {
            //p = vec3(sumX, sumY, sumZ);
            p = vec3(0.0f, 0.0f, 0.0f);
        }
        */

        for (int i = 0; i < subd; i++)
        {
            points[i] = vec3(sumX, sumY, sumZ);

        }
    
    }

};


// The quadLoop takes in two Circles, and generates a vector of quads between them.
class quadLoop
{
public:
    Circle ccw;
    Circle cw;
    vector<quad> quads;
    int numQuads;


    quadLoop(Circle _ccw = Circle(1.0f, 8), Circle _cw = Circle(1.0f, 8))
    {
        // The constructor checks that the two circles have an equal number of vertices.
        if (_ccw.subd != _cw.subd)
        {
            return;
        }
        ccw = _ccw;
        cw = _cw;

        numQuads = cw.subd;


        for (int i = 0; i < numQuads - 1; i++)
        {
            quad temp = quad(ccw.points[i], ccw.points[i + 1], cw.points[i + 1], cw.points[i]);
            quads.push_back(temp);
        }

        quad temp = quad(cw.points[0], cw.points[numQuads - 1], ccw.points[numQuads - 1], ccw.points[0]);
        quads.push_back(temp);
    }

    


};


// Mesh class holds all data for a primitive.
class Mesh {
public:
    vector<vert> verts;
    vector<vec3> points;
    int* indices;

};


// my Geometry Generator class

class GeomeGen
{
public:


    Mesh genPlane(float width, float height)
    {
        Mesh m;

        int x = width / 2.0f;
        int z = height / 2.0f;

        vert a = vec3(-x, 0, z);
        vert b = vec3(x, 0, z);
        vert c = vec3(x, 0, -z);
        vert d = vec3(-x, 0, -z);

        vert A = vert(a);
        vert B = vert(b);
        vert C = vert(c);
        vert D = vert(d);

        //A.u = 0; A.v = 0;
        //B.u = 1; B.v = 0;
        //C.u = 1; C.v = 1;
        //D.u = 0; D.u = 1;

        quad q = quad(A, B, C, D);

 

        q.UVreset();
        
        m.verts.push_back(q.a.c);
        m.verts.push_back(q.a.b);
        m.verts.push_back(q.a.a);

        m.verts.push_back(q.b.c);
        m.verts.push_back(q.b.b);
        m.verts.push_back(q.b.a);


        return m;
    }

    // GenWobble creates the "wobble" shape, a cylinder multiplied by a sine wave.

    Mesh genWobble(float radius, float height, int radSubd, int numLoops, float wobble)
    {
        Mesh m;

        float angle = 360.0f / radSubd;

        float step = height / numLoops;

        vec3 botCen = vec3(0.0f, 0.0f, 0.0f);
        vec3 topCen = vec3(0.0f, height, 0.0f);

        vec3 radi = vec3(radius, 0.0f, 0.0f);

        Circle* rings = new Circle[numLoops + 1];

        Circle top = Circle(radius, radSubd);
        top.translate(vec3(0.0f, height, 0.0f));

        // Generates circles down the length of the cylinder
        for (int i = 0; i < numLoops + 1; i++)
        {
            Circle temp = top;
            temp.translate(vec3(0.0f, i * -1 * step, 0.0f));

            rings[i] = temp;
        }

        // resizes the circles according to the sine of their normalized position along the length.
        for (int i = 0; i < numLoops + 1; i++)
        {
            Circle temp = rings[i];

            for (int k = 0; k < radSubd; k++)
            {
                float completion = (i * step) / height;
                //float wibble = 1.0f + completion * 360.0f * (3.141592654f / 180.0f) * wobble;
                float wibble = completion * 360.0f * (3.141592654f / 180.0f) * wobble;
                temp.points[k].x = temp.points[k].x * 0.2f * (radius + sin(wibble));
                temp.points[k].z = temp.points[k].z * 0.2f * (radius + sin(wibble));
            }



            rings[i] = temp;
        }

        // Then create quadLoops to link all these circles.

        quadLoop* q = new quadLoop[numLoops];

        for (int i = 0; i < numLoops; i++)
        {
            q[i] = quadLoop(rings[i], rings[i + 1]);

        }

        for (int i = 0; i < numLoops; i++)
        {
            for (int j = 0; j < radSubd; j++)
            {
                q[i].quads[j].UVreset();

                m.verts.push_back(q[i].quads[j].a.c);
                m.verts.push_back(q[i].quads[j].a.b);
                m.verts.push_back(q[i].quads[j].a.a);


                m.verts.push_back(q[i].quads[j].b.c);
                m.verts.push_back(q[i].quads[j].b.b);
                m.verts.push_back(q[i].quads[j].b.a);
            }
        }


        //Mesh discTop = genDisc(radius, radSubd);

        tri* topTris = new tri[radSubd];

        for (int i = 0; i < radSubd - 1; i++)
        {
            tri temp = tri(topCen, top.points[i], top.points[i + 1]);
            topTris[i] = temp;
        }

        tri temp = tri(topCen, top.points[radSubd - 1], top.points[0]);
        topTris[radSubd - 1] = temp;

        for (int i = 0; i < radSubd; i++)
        {
            m.verts.push_back(topTris[i].c);
            m.verts.push_back(topTris[i].b);
            m.verts.push_back(topTris[i].a);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////

        tri* botTris = new tri[radSubd];

        for (int i = 0; i < radSubd - 1; i++)
        {
            tri temp = tri(botCen, rings[numLoops].points[i], rings[numLoops].points[i + 1]);
            botTris[i] = temp;
        }

        temp = tri(botCen, rings[numLoops].points[radSubd - 1], rings[numLoops].points[0]);
        botTris[radSubd - 1] = temp;

        for (int i = 0; i < radSubd; i++)
        {
            m.verts.push_back(botTris[i].a);
            m.verts.push_back(botTris[i].b);
            m.verts.push_back(botTris[i].c);
        }

        return m;
    }

    // My cylinder function is not used in this project, but the above wobble function is based on it.

    Mesh genCyl(float radius, float height, int radSubd, int numLoops)
    {
        Mesh m;

        float angle = 360.0f / radSubd;

        float step = height / numLoops;

        vec3 botCen = vec3(0.0f, 0.0f, 0.0f);
        vec3 topCen = vec3(0.0f, height, 0.0f);

        vec3 radi = vec3(radius, 0.0f, 0.0f);

        Circle* rings = new Circle[numLoops + 1];

        Circle top = Circle(radius, radSubd);
        top.translate(vec3(0.0f, height, 0.0f));
        for (int i = 0; i < numLoops + 1; i++)
        {
            Circle temp = top;
            temp.translate(vec3(0.0f, i * -1 * step, 0.0f));
            rings[i] = temp;
        }

        quadLoop* q = new quadLoop[numLoops];

        for (int i = 0; i < numLoops; i++)
        {
            q[i] = quadLoop(rings[i], rings[i + 1]);

        }

        for (int i = 0; i < numLoops; i++)
        {
            for (int j = 0; j < radSubd; j++)
            {
                m.verts.push_back(q[i].quads[j].a.c);
                m.verts.push_back(q[i].quads[j].a.b);
                m.verts.push_back(q[i].quads[j].a.a);


                m.verts.push_back(q[i].quads[j].b.c);
                m.verts.push_back(q[i].quads[j].b.b);
                m.verts.push_back(q[i].quads[j].b.a);
            }
        }


        //Mesh discTop = genDisc(radius, radSubd);

        tri* topTris = new tri[radSubd];

        for (int i = 0; i < radSubd - 1; i++)
        {
            tri temp = tri(topCen, top.points[i], top.points[i + 1]);
            topTris[i] = temp;
        }

        tri temp = tri(topCen, top.points[radSubd - 1], top.points[0]);
        topTris[radSubd - 1] = temp;

        for (int i = 0; i < radSubd; i++)
        {
            m.verts.push_back(topTris[i].c);
            m.verts.push_back(topTris[i].b);
            m.verts.push_back(topTris[i].a);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////

        tri* botTris = new tri[radSubd];

        for (int i = 0; i < radSubd - 1; i++)
        {
            tri temp = tri(botCen, rings[numLoops].points[i], rings[numLoops].points[i + 1]);
            botTris[i] = temp;
        }

        temp = tri(botCen, rings[numLoops].points[radSubd - 1], rings[numLoops].points[0]);
        botTris[radSubd - 1] = temp;

        for (int i = 0; i < radSubd; i++)
        {
            m.verts.push_back(botTris[i].a);
            m.verts.push_back(botTris[i].b);
            m.verts.push_back(botTris[i].c);
        }

        /*
        //vec3 radii =vec3(radius, 0.0f, 0.0f);
        vec3 radii = radi.Yrotate(angle);


        tri bot1 = tri(botCen, radi, radii);

        radi.y = height;
        radii.y = height;

        tri top1 = tri(topCen, radii, radi);

        vec3 q1 = radi;
        vec3 q2 = radii;
        vec3 q3 = radii;
        q3.y -= step;
        vec3 q4 = radi;
        q4.y -= step;

        quad* q = new quad[numLoops];

        for (int i = 0; i < numLoops; i++)
        {
            q[i] = quad(q1, q2, q3, q4);

            q1.y -= step;
            q2.y -= step;
            q3.y -= step;
            q4.y -= step;
        }
        */

        return m;

    }

    // Generates a disc made of tris.
    Mesh genDisc(float radius, int subd)
    {
        Mesh m;

        vec3 center = vec3(0.0f, 0.0f, 0.0f);

        //m.points.push_back(center);
        vector<vec3> circlePoints;

        vec3 first = vec3(radius, 0.0f, 0.0f);

        float angle = 360.0f / subd;

        for (int i = 0; i < subd + 1; i++)
        {
            circlePoints.push_back(first.Yrotate(angle * i));
        }

        for (int i = 1; i < subd; i++)
        {
            m.points.push_back(center);
            m.points.push_back(circlePoints[i]);
            m.points.push_back(circlePoints[i + 1]);

        }


        m.points.push_back(center);
        m.points.push_back(circlePoints[subd]);
        m.points.push_back(circlePoints[1]);



        return m;
    }

    
    // Creates a smooth cone (normals are recalculated so the sides blend together).
    Mesh genCone(float radius, float height, int subd)
    {
        Mesh m = genDisc(radius, subd);

        vec3 tip = vec3(0.0f, height, 0.0f);

        vec3* copyDisc = new vec3[m.points.size()];

        int size = m.points.size();

        for (int i = 0; i < size; i++)
        {
            copyDisc[i] = m.points[i];

        }


        for (int i = 0; i < m.points.size(); i++)
        {
            if (copyDisc[i].x == 0.0f && copyDisc[i].z == 0.0f)
            {
                copyDisc[i] = tip;
            }
        }

        m.points.push_back(tip);


        for (int i = size - 1; i > 0; i--)
        {
            m.points.push_back(copyDisc[i]);

        }

        size = m.points.size();

        for (int i = 0; i < size; i++)
        {
            vert newVert = vert(m.points[i]);
            m.verts.push_back(newVert);


            if (i < size / 2)
            {
                m.verts[i].normal = vec3(0.0f, -1.0f, 0.0f);
            }
            else
            {
                // The normals of these vertices are the vector TO each one from the center, creating the smooth sides.
                if (!(newVert.position.x == 0 && newVert.position.z == 0))
                {
                    vec3 horiz = vec3(0.0f, 0.0f, 0.0f).to(m.points[i]);
                    horiz.y = 0.2f;
                    m.verts[i].normal = horiz;
                }
                else
                {
                    m.verts[i].normal = vec3(0.0f, 1.0f, 0.0f);
                }
            }
        }


        return m;

    }

    // Generates an icosahedron (12-sided polyhedron)
    Mesh genD20(float radius)
    {
        float r1 = radius;
        float r2 = radius * 1.61803f;

        // The vertices of the icosahedron are cyclic permutations of (Phi, 1, 0):

        vec3 p[12] = {
            
            vec3(r2, r1, 0),
            vec3(0, r2, r1),
            vec3(r1, 0, r2),

            vec3(-r2, -r1, 0),
            vec3(0, -r2, -r1),
            vec3(-r1, 0, -r2),

            vec3(r2, -r1, 0),
            vec3(0, r2, -r1),
            vec3(-r1, 0, r2),

            vec3(-r2, r1, 0),
            vec3(0, -r2, r1),
            vec3(r1, 0, -r2),
        };

        vector<tri> faces;

        // These for loops draw a triangle between any three vertices that are equidistant from one another.
        // (This does draw some triangles inside the shape)

        for (int i = 0; i < 12; i++)
        {
            for (int k = 0; k < 12; k++)
            {
                for (int j = 0; j < 12; j++)
                {
                   if (j != k && j != i)
                   {
                        if (abs(p[i].length(p[j]) - p[i].length(p[k])) < 0.1f && abs(p[i].length(p[k]) - p[j].length(p[k])) < 0.1f)
                        {
                            faces.push_back(tri(p[i], p[j], p[k]));
                        }
                   }
                }
            }
        }

        
        Mesh m;

        for (tri t : faces)
        {
            m.verts.push_back(t.c);
            m.verts.push_back(t.b);
            m.verts.push_back(t.a);
        }

        return m;
    }

    Mesh genStepPyr(float stepHeight, float stepWidth)
    {

    }


    // Draws the diamond shape (essentially two pyramid/cones)

    Mesh genDiamond(float radius, float height, int subd)
    {
        Circle center = Circle(radius, subd);

        vec3 top = vec3(0.0f, 0.5f * height, 0.0f);
        vec3 bottom = vec3(0.0f, -0.5f * height, 0.0f);


        vector<tri> tris;

        for (int i = 0; i < subd - 1; i++)
        {
            tri uTemp = tri(top, center.points[i], center.points[i+1]);
            tris.push_back(uTemp);

            tri dTemp = tri(bottom, center.points[i+1], center.points[i]);
            tris.push_back(dTemp);
        }

        tri uTemp = tri(top, center.points[subd-1], center.points[0]);
        tris.push_back(uTemp);

        tri dTemp = tri(bottom, center.points[0], center.points[subd-1]);
        tris.push_back(dTemp);

        Mesh m;

        for (int i = 0; i < tris.size(); i++)
        {
            m.verts.push_back(tris[i].c);
            m.verts.push_back(tris[i].b);
            m.verts.push_back(tris[i].a);

        }
        /*
        for (tri t : tris)
        {
            m.verts.push_back(t.c);
            m.verts.push_back(t.b);
            m.verts.push_back(t.a);
        }
        */


        return m;
    }


    // The gear is a cylinder with certen vertices extended.

    Mesh genGear(float _radius, float _thickness)
    {
        // Starts with a circle
        Circle top = Circle(_radius, 32);

        // scales up x and z coordinates of key vertices
        for (int i = 0; i < 32; i++)
        {
            if (i % 4 == 0 || (i - 1) % 4 == 0)
            {
                top.points[i].x *= 1.4;
                top.points[i].z *= 1.4;
            }
        }

        // copies the top to make the bottom
        Circle bottom = top;
        bottom.translate(vec3(0.0f, -_thickness, 0.0f));


        // generates a quadLoop between these two non-circular circles.
        quadLoop edge = quadLoop(bottom, top);


        // Then build the faces out of triangles.

        vector<tri> topFace;

        vector<tri> bottomFace;

        vec3 topCen = vec3(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < 31; i++)
        {
            tri temp = tri(topCen,  top.points[i + 1], top.points[i] );
            topFace.push_back(temp);
        }

        tri temp = tri(topCen, top.points[0], top.points[31]);
        topFace.push_back(temp);


        vec3 botCen = vec3(0.0f, -_thickness, 0.0f);

        for (int i = 0; i < 31; i++)
        {
            tri temp = tri(botCen, bottom.points[i], bottom.points[i + 1]);
            bottomFace.push_back(temp);
        }

        temp = tri(botCen, bottom.points[31], bottom.points[0]);
        bottomFace.push_back(temp);

        Mesh m;

        for (tri t : topFace)
        {
            m.verts.push_back(t.a);
            m.verts.push_back(t.b);
            m.verts.push_back(t.c);
        }

        for (tri t : bottomFace)
        {
            m.verts.push_back(t.a);
            m.verts.push_back(t.b);
            m.verts.push_back(t.c);
        }

        for (quad q : edge.quads)
        {
            q.UVreset();

            m.verts.push_back(q.a.a);
            m.verts.push_back(q.a.b);
            m.verts.push_back(q.a.c);

            m.verts.push_back(q.b.a);
            m.verts.push_back(q.b.b);
            m.verts.push_back(q.b.c);
        }

        return m;
    }

    // The corkscrew copies and rotates a circle many times, then connects the copies with quadLoops to draw a curlicue.

    Mesh genCorkScrew(float _thickness, float _radius, float _arcLength, int _numArcs, int _subd)
    {
        float thickness = _thickness;
        float radius = _radius;
        float arcLength = _arcLength;
        int numArcs = _numArcs;
        int subd = _subd;

        Circle root = Circle(thickness, subd);
        root = root.xRotate(-30.0f);
        root.translate(vec3(radius, 0.0f, 0.0f));

        vector<Circle> rings;


        while (rings.size() < numArcs)
        {
            Circle temp = root.yRotate(arcLength * rings.size());
            temp.translate(vec3(0.0f, 0.5f * rings.size(), 0.0f));

            rings.push_back(temp);
        }


        rings[0].Collapse();
        rings[numArcs - 1].Collapse();

        vector<quadLoop> q;

        while (q.size() < numArcs - 1)
        {
            quadLoop temp = quadLoop(rings[q.size()], rings[q.size() + 1]);

            q.push_back(temp);
        }


        Mesh m;

        int numVerts = numArcs * subd * 4;

        int numLoops = q.size();

        for (int i = 0; i < numLoops; i++)
        {
            for (int k = 0; k < subd; k++)
            {
                q[i].quads[k].UVreset();

                m.verts.push_back(q[i].quads[k].a.a);
                m.verts.push_back(q[i].quads[k].a.b);
                m.verts.push_back(q[i].quads[k].a.c);

                m.verts.push_back(q[i].quads[k].b.a);
                m.verts.push_back(q[i].quads[k].b.b);
                m.verts.push_back(q[i].quads[k].b.c);

              
            }
        }
                                 
        return m;
    }

    // The torus is my crowning achievement.

    Mesh genTorus(float thickness = 1.0f, float innerRadius = 2.0f, int crossSectionSides = 6.0f, int ringSubD = 8.0f)
    {
        Mesh m;

        
        // Start with a circle.
        Circle ccw = Circle(thickness, crossSectionSides);

        // Then rotate and translate it into position, as a cross-section of the torus.
        ccw = ccw.xRotate(90.0f);        
        ccw.translate(vec3(innerRadius, 0.0f, 0.0f));
        
        // Then copy the first circle and rotate to the end of the first segment.
        Circle cw = ccw.copy();
        cw = cw.yRotate(360.0f / float(ringSubD));

        // Generate the first segment of the torus by creating a quadLoop between these two circles.
        quadLoop firstLoop = quadLoop(ccw, cw);

        // Then copy that loop all the way around.
        vector<quadLoop> loops;

        
        for (int i = 0; i < ringSubD; i++)
        {
            quadLoop temp = quadLoop(firstLoop.ccw.copy(), firstLoop.cw.copy());

            temp.ccw = temp.ccw.yRotate(i * 360.0f / float(ringSubD));
            temp.cw = temp.cw.yRotate(i * 360.0f / float(ringSubD));

            quadLoop temp2 = quadLoop(temp.ccw, temp.cw);


            loops.push_back(temp2);
        }

        //vector<vec3> verts;

        for (quadLoop ql : loops)
        {
            for (quad q : ql.quads)
            {
                q.UVreset();

                m.points.push_back(q.a.c.position);
                m.points.push_back(q.a.b.position);
                m.points.push_back(q.a.a.position);

                m.points.push_back(q.b.c.position);
                m.points.push_back(q.b.b.position);
                m.points.push_back(q.b.a.position);


                m.verts.push_back(q.a.c);
                m.verts.push_back(q.a.b);
                m.verts.push_back(q.a.a);
                  
                m.verts.push_back(q.b.c);
                m.verts.push_back(q.b.b);
                m.verts.push_back(q.b.a);
            }
        }

        return m;
    }


    // A simple cube function, built to test the tri and quad classes.
    Mesh genCube(float sideLength)
    {
        Mesh m;

        float d = 0.5 * sideLength;

        vec3 verts[8];

        // FIRST GENERATE VERTEX COORDINATES

         // Top
        verts[0] = vec3(-d, d, d);
        verts[1] = vec3(d, d, d);
        verts[2] = vec3(d, d, -d);
        verts[3] = vec3(-d, d, -d);

        // Bottom
        verts[4] = vec3(-d, -d, d);
        verts[5] = vec3(d, -d, d);
        verts[6] = vec3(d, -d, -d);
        verts[7] = vec3(-d, -d, -d);

        // THEN CREATE QUADS
        // (Pass the points in CLOCKWISE!!!!!)


        // Top
        quad top = quad(verts[0], verts[1], verts[2], verts[3]);
        // Bottom
        quad bottom = quad(verts[7], verts[6], verts[5], verts[4]);
        // Front
        quad front = quad(verts[3], verts[2], verts[6], verts[7]);
        // Back
        quad back = quad(verts[1], verts[0], verts[4], verts[5]);
        // Left
        quad left = quad(verts[0], verts[3], verts[7], verts[4]);
        // Right
        quad right = quad(verts[2], verts[1], verts[5], verts[6]);

        quad quads[6] = { top, bottom, front, back, left, right };

        int count = 0;
        /*
        while (count < 8)
        {
            vert v;
            v.position = verts[count];
            v.index = count;
            m.verts.push_back(v);

            count++;
        }
        */

        
            for (quad q : quads)
            {
                q.UVreset();

                m.points.push_back(q.a.c.position);
                m.points.push_back(q.a.b.position);
                m.points.push_back(q.a.a.position);

                m.points.push_back(q.b.c.position);
                m.points.push_back(q.b.b.position);
                m.points.push_back(q.b.a.position);

                m.verts.push_back(q.a.c);
                m.verts.push_back(q.a.b);
                m.verts.push_back(q.a.a);

                m.verts.push_back(q.b.c);
                m.verts.push_back(q.b.b);
                m.verts.push_back(q.b.a);
            }
        

        return m;

    }

};