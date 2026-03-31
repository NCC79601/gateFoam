#ifndef ISHAPE_H
#define ISHAPE_H

#include "../types.h"
#include "./sdf/sdf.h"
#include <algorithm>
#include <memory>
namespace sdfibm {

#define SHAPETYPENAME(name) \
    static std::string typeName() {return name;} \
    static bool added; \
    virtual std::string getTypeName() const {return name;}

class IShape;

template <typename T>
class _shapecreator
{
public:
    static std::unique_ptr<IShape> create(const dictionary& para)
    {
        return std::make_unique<T>(para);
    }
};

class IShape
{
public:
    struct Transformation {vector t; quaternion q;};
    const static int m_id = -1;
    scalar m_radiusB   {0.0}; // radius of bounding sphere
    scalar m_volume    {0.0};
    scalar m_volumeINV {0.0}; 
    vector m_com    {vector::zero};
    tensor m_moi    {tensor::I}; // in principal frame, diagonal
    tensor m_moiINV {tensor::I};
    bool finite {true};

public:
    SHAPETYPENAME("IShape");
    
    inline static vector world2local(const vector& p, const Transformation& tr)
    {
        // FIXME: THIS IS THE CAUSE!!!
        // return Foam::conjugate(tr.q).transform(p - tr.t);
        // workaround:
        // return p - tr.t;
        return tr.q.R().T() & (p - tr.t);
    }

    virtual int    getShapeID() const {return m_id;}
    virtual scalar getRadiusB() const {return m_radiusB;}

    bool phi01(const vector& p, const Transformation& tr) { return isInside      (world2local(p, tr)); }
    scalar phi(const vector& p, const Transformation& tr) {
        // FIXME:
        #ifdef GATEFOAM_DEBUG
        // debug info
        {
            // printf(">>>>> IShape::phi(p, tr) receiving p=(%lf, %lf, %lf)\n", p.x(), p.y(), p.z());
            // vector w2l_p= world2local(p, tr);
            // printf("      world2local_p = (%lf, %lf, %lf)\n", w2l_p.x(), w2l_p.y(), w2l_p.z());
            vector local_p = world2local(p, tr);
            
            printf(">>> IShape::phi(p, tr):\n");
            printf("    p = (%.6f, %.6f, %.6f)\n", p.x(), p.y(), p.z());
            printf("    tr.t = (%.6f, %.6f, %.6f)\n", tr.t.x(), tr.t.y(), tr.t.z());
            // printf("    tr.q = (%.6f, %.6f, %.6f, %.6f)\n", 
                // tr.q.w(), tr.q.x(), tr.q.y(), tr.q.z());
            // printf("    conjugate(tr.q) = (%.6f, %.6f, %.6f, %.6f)\n",
                // q_conj.w(), q_conj.x(), q_conj.y(), q_conj.z());
            printf("    local_p = (%.6f, %.6f, %.6f)\n", local_p.x(), local_p.y(), local_p.z());
            printf("\n");
        }
        #endif
        
        return signedDistance(world2local(p, tr));
    }

    virtual std::string description() const = 0;

    virtual ~IShape(){}

private:
    virtual bool   isInside      (const vector& p) const = 0; // local coordinate
    virtual scalar signedDistance(const vector& p) const = 0; // local coordinate
};

}
#endif
