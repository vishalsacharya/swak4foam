/*---------------------------------------------------------------------------*\
 ##   ####  ######     |
 ##  ##     ##         | Copyright: ICE Stroemungsfoschungs GmbH
 ##  ##     ####       |
 ##  ##     ##         | http://www.ice-sf.at
 ##   ####  ######     |
-------------------------------------------------------------------------------
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright  held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is based on OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Contributors/Copyright:
    2011, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

 SWAK Revision: $Id:  $ 
\*---------------------------------------------------------------------------*/

#include "solvePDECommonFiniteArea.H"

#include "polyMesh.H"

#include "areaFields.H"

#include "FaFieldValueExpressionDriver.H"

#include "faScalarMatrix.H"

#include "fvm.H"

namespace Foam {
    defineTypeNameAndDebug(solvePDECommonFiniteArea,0);
}

Foam::solvePDECommonFiniteArea::solvePDECommonFiniteArea
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
):
    solvePDECommon(
        name,
        obr,
        dict,
        loadFromFiles
    )
{
    if (!isA<polyMesh>(obr))
    {
        active_=false;
        WarningIn("solvePDECommonFiniteArea::solvePDECommonFiniteArea")
            << "Not a polyMesh. Nothing I can do"
                << endl;
    }
}

Foam::solvePDECommonFiniteArea::~solvePDECommonFiniteArea()
{}

void Foam::solvePDECommonFiniteArea::read(const dictionary& dict)
{
    solvePDECommon::read(dict);

    if(active_) {
        const fvMesh& mesh = refCast<const fvMesh>(obr_);
        
        if(
            theField_.valid()
            &&
            fieldName_!=theField_->name()
        ) {
            WarningIn("Foam::solvePDECommonFiniteArea::read(const dictionary& dict)")
                << "Throwing out field " << theField_->name()
                    << " and loading " << fieldName_ << ". "
                    << "This might lead to unpredicatable behaviour" << endl;
            theField_.clear();
        }

        driver_.set(
            new FaFieldValueExpressionDriver(
                mesh,
                false, // no caching. No need
                true,  // search fields in memory
                false  // don't look up files on disc
            )
        );
        
        if(!theField_.valid()) {
            theField_.set(
                new areaScalarField(
                    IOobject (
                        fieldName_,
                        mesh.time().timeName(),
                        mesh,
                        IOobject::MUST_READ,
                        IOobject::AUTO_WRITE
                    ),
                    driver_->aMesh()
                )
            );
        }

        driver_->readVariablesAndTables(dict);
        
        driver_->createWriterAndRead(name_+"_"+fieldName_+"_"+type());
    }
}

void Foam::solvePDECommonFiniteArea::writeData()
{
    theField_->write();
    
    driver_->tryWrite();
}

// ************************************************************************* //
