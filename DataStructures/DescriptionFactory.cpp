/*
 open source routing machine
 Copyright (C) Dennis Luxen, others 2010

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU AFFERO General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 or see http://www.gnu.org/licenses/agpl.txt.
 */

#include <boost/foreach.hpp>

#include "../typedefs.h"
#include "DescriptionFactory.h"

DescriptionFactory::DescriptionFactory() { }

DescriptionFactory::~DescriptionFactory() { }

double DescriptionFactory::GetAngleBetweenCoordinates() const {
    return 0.;//GetAngleBetweenTwoEdges(previousCoordinate, currentCoordinate, nextCoordinate);
}

void DescriptionFactory::SetStartSegment(const PhantomNode & _startPhantom) {
    startPhantom = _startPhantom;
    AppendSegment(_startPhantom.location, _PathData(0, _startPhantom.nodeBasedEdgeNameID, 10, _startPhantom.weight1));
}

void DescriptionFactory::SetEndSegment(const PhantomNode & _targetPhantom) {
    targetPhantom = _targetPhantom;
    AppendSegment(_targetPhantom.location, _PathData(0, _targetPhantom.nodeBasedEdgeNameID, 0, _targetPhantom.weight1));
}

void DescriptionFactory::AppendSegment(const _Coordinate & coordinate, const _PathData & data ) {
    //Segment information has following format:
    //["instruction","streetname",length,position,time,"length","earth_direction",azimuth]
    //Example: ["Turn left","High Street",200,4,10,"200m","NE",22.5]
    //See also: http://developers.cloudmade.com/wiki/navengine/JSON_format
//    (_Coordinate & loc, NodeID nam, unsigned len, unsigned dur, short tInstr)

    //Is a new instruction necessary?
    //yes: data.turnInstruction != 0;
    //no: data.turnInstruction == 0;
    pathDescription.push_back(SegmentInformation(coordinate, data.nameID, 0, data.durationOfSegment, data.turnInstruction) );

}

void DescriptionFactory::AppendRouteInstructionString(std::string & output) {
    output += "[\"Turn left\",\"High Street\",200,0,10,\"200m\",\"NE\",22.5]";
}

void DescriptionFactory::AppendEncodedPolylineString(std::string & output, bool isEncoded) {
    if(isEncoded)
        pc.printEncodedString(pathDescription, output);
    else
        pc.printUnencodedString(pathDescription, output);
}

void DescriptionFactory::AppendEncodedPolylineString(std::string &output) {
    pc.printEncodedString(pathDescription, output);
}

void DescriptionFactory::AppendUnencodedPolylineString(std::string &output) {
    pc.printUnencodedString(pathDescription, output);
}

unsigned DescriptionFactory::Run() {
    unsigned entireLength = 0;
    /** starts at index 1 */
    pathDescription[0].length = 0;
    for(unsigned i = 1; i < pathDescription.size(); ++i) {
        pathDescription[i].length = ApproximateDistance(pathDescription[i-1].location, pathDescription[i].location);
    }

    unsigned lengthOfSegment = 0;
    unsigned durationOfSegment = 0;
    unsigned indexOfSegmentBegin = 0;

    for(unsigned i = 1; i < pathDescription.size(); ++i) {
        entireLength += pathDescription[i].length;
        lengthOfSegment += pathDescription[i].length;
        durationOfSegment += pathDescription[i].duration;
        pathDescription[indexOfSegmentBegin].length = lengthOfSegment;
        pathDescription[indexOfSegmentBegin].duration = durationOfSegment;
        if(pathDescription[i].turnInstruction != 0) {
            //INFO("Turn after " << lengthOfSegment << "m into way with name id " << segment.nameID);
            assert(pathDescription[i].necessary);
            lengthOfSegment = 0;
            durationOfSegment = 0;
            indexOfSegmentBegin = i;
        }
    }

    //Generalize poly line
    BOOST_FOREACH(SegmentInformation & segment, pathDescription) {
        //TODO: Replace me by real generalization
        segment.necessary = true;
    }

    //fix what needs to be fixed else
    return entireLength;
}