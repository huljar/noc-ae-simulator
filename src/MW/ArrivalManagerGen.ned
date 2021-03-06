//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package HaecComm.MW;

//
// ArrivalManager module for the full-generation authentication protocol.
//
// This module manages flits arriving from the network, including their forwarding to the crypto modules,
// verification, and ARQ issuance.
//
simple ArrivalManagerGen like IArrivalManager {
    parameters:
        @signal[generateArq](type=HaecComm::Messages::Flit);
        @statistic[arqsGenerated](source=generateArq; record=vector(flitId),vector(flitTarget)?; title="Generated ARQs");
        @display("i=block/cogwheel");
        int arqLimit;
        int arqIssueTimeout;
        int arqAnswerTimeoutBase;
        bool lastArqWaitForOngoingVerifications;
        int finishedIdsTracked = default(100);
    gates:
        input  netIn;
        input  cryptoIn;
        output appOut;
        output decoderOut;
        output arqOut;
}
