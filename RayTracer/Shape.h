#ifndef __HW__SHAPE__
#define __HW__SHAPE__

#include <iostream>
#include <vector>
#include <math.h>
#include "Vector.h"
#include "Intersection.h"

struct Face
{
    int v0_id;
    int v1_id;
    int v2_id;
};

struct Shape
{
    virtual bool intersect(Intersection& i, const std::vector<Vec3f> &vertex_data, bool backface_culling) = 0;

    float determinant_2(float a, float b, float c, float d)
    {
        return a*d - b*c;
    }
    
    float determinant_3(Vec3f row1, Vec3f row2, Vec3f row3)
    {
        return row1.x * determinant_2(row2.y, row2.z, row3.y, row3.z) - row1.y * determinant_2(row2.x, row2.z, row3.x, row3.z)
            + row1.z * determinant_2(row2.x, row2.y, row3.x, row3.y);
    }
};

struct Sphere: public Shape
{
    int material_id;
    int center_vertex_id;
    float radius;

    bool intersect(Intersection &i, const std::vector<Vec3f> &vertex_data, bool backface_culling)
    {
        Point center = vertex_data[center_vertex_id];

        float root1, root2;
        Ray ray = i.ray;

        Vec3f direction = ray.direction;
        Vec3f distance_vector = ray.origin - center;
                    
        float A = direction.dot(direction);
        float B = direction.dot(distance_vector) * 2;
        float C = distance_vector.dot(distance_vector) - radius * radius;
        
        float discriminant = B * B - 4 * A * C;
        if(discriminant < 0) return false;
        else
        {
            discriminant = sqrt(discriminant);
            root1 = (-B + discriminant)/(2*A);
            root2 = (-B - discriminant)/(2*A); 
        }
            
        float small = root1 < root2 ? root1 : root2;
        float big = root1 > root2 ? root1 : root2;

        if(small < -1e-6)
        {
            if(backface_culling)
            {
                return false;
            }
            else if(big < -1e-6)
            {
                return false;
            }
            i.t = big;
        }

        if(i.t > small) 
        {
            i.t = small;
        }

        i.material_id = material_id;
        i.surfaceNormal = (ray.calculate(i.t) - center).normalized();
        return true;
    }
};

struct Mesh
{
    int material_id;
    std::vector<Face> faces;

};

struct Triangle: public Shape
{
    int material_id;
    Face indices;

    bool intersect(Intersection &i, const std::vector<Vec3f> &vertex_data, bool backface_culling)
    {
        Ray ray = i.ray;
        Vec3f direction = i.ray.direction;

        Point a = vertex_data[indices.v0_id];
        Point b = vertex_data[indices.v1_id];
        Point c = vertex_data[indices.v2_id];
        
        Vec3f normal = ((b-a).cross(c-a)).normalized();

        if(backface_culling && direction.dot(normal) > 0) return false;

        float A = determinant_3(Vec3f(a.x - b.x, a.x - c.x, direction.x),
                                    Vec3f(a.y - b.y, a.y - c.y, direction.y),
                                    Vec3f(a.z - b.z, a.z - c.z, direction.z));

        float beta = determinant_3(Vec3f(a.x - ray.origin.x, a.x - c.x, direction.x),
                                    Vec3f(a.y - ray.origin.y, a.y - c.y, direction.y),
                                    Vec3f(a.z - ray.origin.z, a.z - c.z, direction.z)) / A;
        if(beta < -1e-6) return false;

        float gamma = determinant_3(Vec3f(a.x - b.x, a.x - ray.origin.x, direction.x),
                                    Vec3f(a.y - b.y, a.y - ray.origin.y, direction.y),
                                    Vec3f(a.z - b.z, a.z - ray.origin.z, direction.z)) / A;
        if(gamma < -1e-6) return false;
        if(beta + gamma > 1) return false;

        float t = determinant_3(Vec3f(a.x - b.x, a.x - c.x, a.x - ray.origin.x),
                                    Vec3f(a.y - b.y, a.y - c.y, a.y - ray.origin.y),
                                    Vec3f(a.z - b.z, a.z - c.z, a.z - ray.origin.z)) / A;

        if(t > 0 && i.t > t)
        {
            i.t = t;
        }
        else
        {
            return false;
        }

        i.material_id = material_id;
        i.surfaceNormal = normal;
        return true;
    }
};

#endif