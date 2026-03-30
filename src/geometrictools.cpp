#include "geometrictools.h"
#include "solid.h"

namespace sdfibm {

scalar GeometricTools::UpdateCache(label vertexInd, const Solid& solid)
{
    if (phiCache.find(vertexInd) == phiCache.end()) {
        phiCache[vertexInd] = solid.phi(m_pp[vertexInd]);
        #ifdef GATEFOAM_DEBUG
        // FIXME: debug info
        {
            printf(">>>>> GeometricTools::UpdateCache():\n");
            printf("      now visiting m_pp[%d] = (%lf, %lf, %lf), phiCache[%d]=%lf\n", vertexInd, m_pp[vertexInd].x(), m_pp[vertexInd].y(), m_pp[vertexInd].z(), vertexInd, phiCache[vertexInd]);
        }
        #endif
    }
    return phiCache[vertexInd];
}

scalar GeometricTools::calcLineFraction(const scalar& phia, const scalar& phib) const
{
    if (phia > 0 && phib > 0)
        return 0;
    if (phia <= 0 && phib <= 0)
        return 1;
    if (phia > 0) // phib < 0
        return -phib/(phia-phib);
    else
        return -phia/(phib-phia);
}

vector GeometricTools::calcApex(const Foam::labelList& vertexInds, CacheMap& phis) const
{
    label nvertex = vertexInds.size(); // #face vertex

    const vector& A = m_pp[vertexInds[0]];
    scalar     phiA = phis[vertexInds[0]];

    vector B = vector::zero;
    scalar phiB = 0.0;
    label i;
    for (i = 1; i < nvertex; ++i)
    {
        B    = m_pp[vertexInds[i]];
        phiB = phis[vertexInds[i]];

        if (phiA * phiB <= 0)
            break;
    }

    return A - std::abs(phiA)/(SMALL + std::fabs(phiA)+std::fabs(phiB))*(A-B);
}

scalar GeometricTools::calcCellVolume(label cellInd, const Solid& solid, bool isTWOD = false)
{
    const Foam::labelList& vertexInds = m_c2p[cellInd];
    
    // prepare cell's vertex phi
    forAll(vertexInds, ivertex)
    {
        UpdateCache(vertexInds[ivertex], solid);
    }
 
    vector apex = calcApex(vertexInds, phiCache);
    if (isTWOD)
        apex[2] = 0.0;
 
    scalar volume = 0.0;
    const Foam::cell& faceInds = m_mesh.cells()[cellInd];
    
    forAll(faceInds, iface)
    {
        label faceInd = faceInds[iface];
        Foam::face myface = m_mesh.faces()[faceInd];
        Foam::scalar eps_f = calcFaceAreaFraction(myface, phiCache, faceInd);
        
        #ifdef GATEFOAM_DEBUG
        // debug info
        {
            printf(" > [DEBUG] Face %d area fraction: %.6f\n", iface, eps_f);
        }
        #endif
        
        volume += (1.0/3.0)*eps_f*std::fabs((apex - m_fc[faceInd]) & m_fa[faceInd]);
    }
    
    #ifdef GATEFOAM_DEBUG
    // debug info
    {
        printf(" > [DEBUG] Cell %d: volume = %.6e, cv = %.6e, ratio = %.6f\n", 
               cellInd, volume, m_mesh.V()[cellInd], volume/m_mesh.V()[cellInd]);
    }
    #endif
    
    return volume;
}

scalar GeometricTools::calcFaceArea(const Foam::face& vertexInds, CacheMap& phis)
{
    label nvertex = vertexInds.size(); // #face vertex

    vector apex = calcApex(vertexInds, phis);

    std::vector<scalar> phiarr(nvertex);
    forAll(vertexInds, ivertex)
    {
        phiarr[ivertex] = phis[vertexInds[ivertex]];
    }

    scalar area = 0.0;
    for (int iseg = 0; iseg < nvertex; ++iseg)
    {
        const scalar& phiO = phiarr[iseg];
        const scalar& phiA = phiarr[(iseg+1)%nvertex];
        const vector& O = m_pp[vertexInds[iseg]];
        const vector& A = m_pp[vertexInds[(iseg+1)%nvertex]];
        area += std::fabs(0.5*Foam::mag((A-O) ^ (apex-O))) * calcLineFraction(phiO, phiA);
    }
    return area;
}

scalar GeometricTools::calcFaceAreaFraction(const Foam::face& vertexInds, CacheMap& phis, label faceInd)
{
    label nvertex = vertexInds.size();
    int sign_sum = 0;

    #ifdef GATEFOAM_DEBUG
    // debug info: 打印前几个面的 phi 值详情
    if (faceInd < 10 && Foam::Pstream::master()) {
        printf("Face %d vertex count: %d\n", faceInd, nvertex);
        forAll(vertexInds, ivertex) {
            printf(" > [calcFaceArea DEBUG] vertex %d: phi = %.6e, pos = (%.3f, %.3f, %.3f)\n", 
                   ivertex, phis[vertexInds[ivertex]], 
                   m_pp[vertexInds[ivertex]].x(),
                   m_pp[vertexInds[ivertex]].y(),
                   m_pp[vertexInds[ivertex]].z());
        }
    }
    #endif
    
    forAll(vertexInds, ivertex)
    {
        if (phis[vertexInds[ivertex]] > 0)
            ++sign_sum;
        else
            --sign_sum;
    }
    
    #ifdef GATEFOAM_DEBUG
    // debug info
    if (faceInd % 5 == 0) {
        printf("  Face %d sign_sum: %d (nvert=%d)\n", faceInd, sign_sum, nvertex);
    }
    #endif
 
    if (sign_sum == nvertex) // ALL_OUT
        return 0.0;
    if (sign_sum ==-nvertex) // ALL_IN
        return 1.0;
 
    scalar calculated_area = calcFaceArea(vertexInds, phis);
    scalar face_area = Foam::mag(m_fa[faceInd]);
    
    #ifdef GATEFOAM_DEBUG
    // debug info
    if (faceInd % 5 == 0) {
        printf(" > [DEBUG] Face %d: calc_area=%.6e, face_area=%.6e, fraction=%.6f\n", 
               faceInd, calculated_area, face_area, calculated_area/face_area);
    }
    #endif
    
    return calculated_area/face_area;
}

}