// modified from interFoam.C

/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.1
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Application
    interFoam

Description
    Solver for 2 incompressible, isothermal immiscible fluids using a VOF
    (volume of fluid) phase-fraction based interface capturing approach.

    The momentum and other fluid properties are of the "mixture" and a single
    momentum equation is solved.

    Turbulence modelling is generic, i.e.  laminar, RAS or LES may be selected.

    For a two-fluid approach see twoPhaseEulerFoam.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "MULES.H"
#include "subCycle.H"
#include "interfaceProperties.H"
#include "twoPhaseMixture.H"
#include "turbulenceModel.H"
#include "pimpleControl.H"

#include "solidcloud.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// #define GATEFOAM_DEBUG

// NOTE: activate ibm interaction here!
#define ENABLE_IBM

int main(int argc, char *argv[])
{
#   include "setRootCase.H"
#   include "createTime.H"
#   include "createMesh.H"

    pimpleControl pimple(mesh);

#   include "readGravitationalAcceleration.H"
#   include "initContinuityErrs.H"
#   include "createFields.h"
#   include "createTimeControls.H"
#   include "correctPhi.H"
#   include "CourantNo.H"
#   include "setInitialDeltaT.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    // initialize sdfibm
    
    std::string dictfile;

    // if start-time > 0, read from start-time-folder for solidDict, otherwise read from case root
    if (runTime.time().value() > 0)
    {
        if (!Foam::Pstream::parRun())
        {
            dictfile = mesh.time().timeName() + "/solidDict";
        }
        else
        {
            dictfile = "processor0/" + mesh.time().timeName() + "/solidDict";
        }
    }
    else
    {
        dictfile = "solidDict";
    }

    sdfibm::SolidCloud solidcloud(runTime.path() + "/" + dictfile, U, runTime.value());
    solidcloud.saveState(); // write the initial condition

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.run())
    {
#       include "readTimeControls.H"
#       include "CourantNo.H"
#       include "setDeltaT.H"

        runTime++;

        Info<< "Time = " << runTime.timeName() << nl << endl;

        Foam::dimensionedScalar dt = runTime.deltaT();

        // calculate source terms introduced by solid interaction
        // #ifdef ENABLE_IBM
        // solidcloud.interact(runTime.value(), dt.value());
        // #endif

        // Pressure-velocity corrector
        while (pimple.loop())
        {
            twoPhaseProperties.correct();

#           include "alphaEqnSubCycle.H"

            // calculate source terms introduced by solid interaction
            // TODO: maybe only once?
            #ifdef ENABLE_IBM
            // solidcloud.interact(runTime.value(), dt.value());
            #endif

#           include "UEqn.H"

            // --- PISO loop
            while (pimple.correct())
            {
                // IBM correction is implemented in pEqn.H
#               include "pEqn.H"
            }

#           include "continuityErrs.H"

            p = pd + rho*gh;

            if (pd.needReference())
            {
                p += dimensionedScalar
                (
                    "p",
                    p.dimensions(),
                    pRefValue - getRefCellValue(p, pdRefCell)
                );
            }

            turbulence->correct();
        }

        // TODO: maybe once? does it need iterative correction?
        
        // deprecated: apply IBM correction
        /*
        solidcloud.interact(runTime.value(), dt.value());
        
        U = U - Fs * dt;
        phi = phi - dt * (linearInterpolate(Fs) & mesh.Sf());
        U.correctBoundaryConditions();
        adjustPhi(phi, U, pd); // pd?
        */

        #ifdef ENABLE_IBM
        // evolve the solidcloud
        solidcloud.evolve(runTime.value(), dt.value());
        solidcloud.saveState();

        // FIXME: seems problematic
        // solidcloud.fixInternal(dt.value());
        #endif
        
        if (runTime.outputTime())
        {
            runTime.write();
            
            #ifdef ENABLE_IBM
            if (Foam::Pstream::master())
            {
                std::string file_name;
                if (Foam::Pstream::parRun())
                    file_name = "./processor0/" + runTime.timeName() + "/solidDict";
                else
                    file_name = "./" + runTime.timeName() + "/solidDict";
                solidcloud.saveRestart(file_name);
            }
            #endif
        }

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
