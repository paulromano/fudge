/*
# <<BEGIN-copyright>>
# Copyright (c) 2016, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
# Written by the LLNL Nuclear Data and Theory group
#         (email: mattoon1@llnl.gov)
# LLNL-CODE-683960.
# All rights reserved.
# 
# This file is part of the FUDGE package (For Updating Data and 
#         Generating Evaluations)
# 
# When citing FUDGE, please use the following reference:
#   C.M. Mattoon, B.R. Beck, N.R. Patel, N.C. Summers, G.W. Hedstrom, D.A. Brown, "Generalized Nuclear Data: A New Structure (with Supporting Infrastructure) for Handling Nuclear Data", Nuclear Data Sheets, Volume 113, Issue 12, December 2012, Pages 3145-3171, ISSN 0090-3752, http://dx.doi.org/10. 1016/j.nds.2012.11.008
# 
# 
#     Please also read this link - Our Notice and Modified BSD License
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the disclaimer below.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the disclaimer (as noted below) in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of LLNS/LLNL nor the names of its contributors may be used
#       to endorse or promote products derived from this software without specific
#       prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
# THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
# Additional BSD Notice
# 
# 1. This notice is required to be provided under our contract with the U.S.
# Department of Energy (DOE). This work was produced at Lawrence Livermore
# National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.
# 
# 2. Neither the United States Government nor Lawrence Livermore National Security,
# LLC nor any of their employees, makes any warranty, express or implied, or assumes
# any liability or responsibility for the accuracy, completeness, or usefulness of any
# information, apparatus, product, or process disclosed, or represents that its use
# would not infringe privately-owned rights.
# 
# 3. Also, reference herein to any specific commercial products, process, or services
# by trade name, trademark, manufacturer or otherwise does not necessarily constitute
# or imply its endorsement, recommendation, or favoring by the United States Government
# or Lawrence Livermore National Security, LLC. The views and opinions of authors expressed
# herein do not necessarily state or reflect those of the United States Government or
# Lawrence Livermore National Security, LLC, and shall not be used for advertising or
# product endorsement purposes.
# 
# <<END-copyright>>
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <nfut_utilities.h>
#include <ptwXY.h>
#include <nf_utilities.h>
#include <ptwXY_utilities.h>

static int verbose = 0;
static char *fmtX = "%18.11e\n", *fmtXY = "%18.11e %18.11e\n";

static ptwXPoints *getGroupBoundaries( statusMessageReporting *smr, int align, double xMax );
static ptwXYPoints *getFluxData( statusMessageReporting *smr );
static ptwXYPoints *getCrossSectionData( statusMessageReporting *smr );
static void writeXYDataOnVerbosity( ptwXYPoints *data, const char * const fileName );
static void writeXDataOnVerbosity( statusMessageReporting *smr, ptwXPoints *data, const char * const fileName );
/*
************************************************************
*/
int main( int argc, char **argv ) {

    int iarg, errCount = 0, echo = 0, align = 0;
    double xMin, gMin, gMax;
    ptwXYPoints *flux, *crossSection;
    ptwXPoints *groupBoundaries, *crossSectionGrouped_None;
    statusMessageReporting smr;

    smr_initialize( &smr, smr_status_Ok );

    for( iarg = 1; iarg < argc; iarg++ ) {
        if( strcmp( "-v", argv[iarg] ) == 0 ) {
            verbose = 1; }
        else if( strcmp( "-e", argv[iarg] ) == 0 ) {
            echo = 1; }
        else if( strcmp( "-a", argv[iarg] ) == 0 ) {
            align = 1; }
        else {
            nfu_printErrorMsg( "ERROR %s: invalid input option '%s'", __FILE__, argv[iarg] );
        }
    }
    if( echo ) {
        if( align ) {
            printf( "%s -a\n", __FILE__ ); }
        else {
            printf( "%s\n", __FILE__ );
        }
    }

    crossSection = getCrossSectionData( &smr );
    writeXYDataOnVerbosity( crossSection, "crossSection2.dat" );
    xMin = ptwXY_getPointAtIndex_Unsafely( crossSection, 0 )->x;

    groupBoundaries = getGroupBoundaries( &smr, align, xMin );
    writeXDataOnVerbosity( &smr, groupBoundaries, "groupBoundaries2.dat" );

    flux = getFluxData( &smr );
    writeXYDataOnVerbosity( flux, "flux2.dat" );

    if( ( crossSectionGrouped_None = ptwXY_groupTwoFunctions( &smr, crossSection, flux, groupBoundaries, ptwXY_group_normType_none, NULL ) ) == NULL )
        nfut_printSMRErrorExit2p( &smr, "Via." );
    writeXDataOnVerbosity( &smr, crossSectionGrouped_None, "crossSection_None2.dat" );

    if( ptwX_range( &smr, crossSectionGrouped_None, &gMin, &gMax ) == nfu_Okay ) {
        if( gMin != 0 ) nfu_printErrorMsg( "ERROR %s: gMin = %e != 0", __FILE__, gMin );
        if( gMax != 0 ) nfu_printErrorMsg( "ERROR %s: gMax = %e != 0", __FILE__, gMax ); }
    else {
        if( gMin != 0 ) nfut_printSMRErrorExit2p( &smr, "Via." );
    }

    ptwX_free( crossSectionGrouped_None );
    ptwX_free( groupBoundaries );
    ptwXY_free( flux );
    ptwXY_free( crossSection );

    exit( errCount );
}
/*
************************************************************
*/
static ptwXPoints *getGroupBoundaries( statusMessageReporting *smr, int align, double xMax ) {

    double data[] = {  /* Last points must be last than first point of cross section data. */
        1.30680e-09, 2.09080e-08, 1.30680e-07, 3.34530e-07, 1.17610e-06, 2.09080e-06, 5.65780e-06, 1.30680e-05, 2.07460e-05, 5.12300e-05,
        1.02450e-04, 2.09080e-04, 3.81050e-04, 5.65780e-04, 7.15580e-04, 1.05850e-03, 1.30680e-03, 1.88170e-03, 2.94020e-03, 3.34530e-03,
        4.23390e-03, 5.76280e-03, 7.52700e-03, 1.02450e-02, 1.51060e-02, 2.09080e-02, 2.64620e-02, 3.26690e-02, 3.95300e-02, 7.00200e-02,
        9.89090e-02, 1.30680e-01, 1.81950e-01, 2.07460e-01, 2.41700e-01, 2.70970e-01, 2.94020e-01, 3.34530e-01, 3.77650e-01, 5.12300e-01,
        6.32470e-01, 7.52700e-01, 8.83370e-01, 1.02450e+00, 1.17610e+00, 1.33810e+00, 1.51060e+00, 1.69360e+00, 2.09080e+00, 2.30510e+00,
        2.52990e+00, 2.74110e+00, 3.01080e+00, 3.53350e+00, 4.06880e+00, 4.39600e+00, 4.70440e+00, 4.99080e+00, 5.35250e+00, 5.65780e+00,
        6.04250e+00, 6.36660e+00, 6.73670e+00, 7.15580e+00, 7.54790e+00, 7.90960e+00, 8.32150e+00, 8.78670e+00, 9.17670e+00, 9.66480e+00 };
    int nData = sizeof( data ) / sizeof( double );
    ptwXPoints *groupBoundaries;

    if( align != 0 ) data[nData-1] = xMax;

    if( ( groupBoundaries = ptwX_create( smr, nData, nData, data ) ) == NULL ) 
        nfut_printSMRErrorExit2p( smr, "Via." );
    return( groupBoundaries );
}
/*
************************************************************
*/
static ptwXYPoints *getFluxData( statusMessageReporting *smr ) {

    double data[] = { 0.0000000e+00, 85., 21., 85. };
    int nData = sizeof( data ) / ( 2 * sizeof( double ) );
    ptwXYPoints *flux;

    if( ( flux = ptwXY_create( smr, ptwXY_interpolationLinLin, NULL, 6, 1e-3, 10, 10, nData, data, 0 ) ) == NULL ) 
        nfut_printSMRErrorExit2p( smr, "Via." );
    return( flux );
}
/*
************************************************************
*/
static ptwXYPoints *getCrossSectionData( statusMessageReporting *smr ) {

    double data[] = {
        9.71901000e+00,   1.95442000e-21, 1.00000000e+01,   0.00000000e+00, 1.00606300e+01,   3.21609400e-05, 1.05000000e+01,   2.65223000e-04,
        1.05924100e+01,   3.30958400e-04, 1.05924600e+01,   3.30994000e-04, 1.06370100e+01,   3.62684400e-04, 1.09765300e+01,   6.75135500e-04,
        1.10000000e+01,   7.02675000e-04, 1.10301500e+01,   7.39245600e-04, 1.15000000e+01,   1.46571000e-03, 1.16758100e+01,   1.79692200e-03,
        1.17246000e+01,   1.88883900e-03, 1.18686200e+01,   2.16046100e-03, 1.19895400e+01,   2.38812700e-03, 1.20000000e+01,   2.40779000e-03,
        1.20552800e+01,   2.51162800e-03, 1.20552900e+01,   2.51164700e-03, 1.21061600e+01,   2.60705500e-03, 1.24721800e+01,   3.29353300e-03,
        1.24900000e+01,   3.32695500e-03, 1.25000000e+01,   3.34571000e-03, 1.26858300e+01,   3.74179500e-03, 1.27251700e+01,   3.83271600e-03,
        1.27251800e+01,   3.83273800e-03, 1.28522800e+01,   4.10912400e-03, 1.30000000e+01,   4.43035000e-03, 1.30800300e+01,   4.55648000e-03,
        1.32431000e+01,   4.81348500e-03, 1.34907000e+01,   5.20371300e-03, 1.35000000e+01,   5.21837000e-03, 1.35749800e+01,   5.35443300e-03,
        1.37185900e+01,   5.61503700e-03, 1.37557800e+01,   5.68252400e-03, 1.38359100e+01,   5.82793200e-03, 1.39665800e+01,   6.06505400e-03,
        1.39829100e+01,   6.09468700e-03, 1.40000000e+01,   6.12570000e-03, 1.40526600e+01,   6.16024500e-03, 1.40742400e+01,   6.17440100e-03,
        1.41923800e+01,   6.25190100e-03, 1.41985400e+01,   6.25594200e-03, 1.41985500e+01,   6.25594900e-03, 1.42478000e+01,   6.28825700e-03,
        1.45000000e+01,   6.45370000e-03, 1.50000000e+01,   6.81335000e-03, 1.54919000e+01,   6.96432400e-03, 1.55000000e+01,   6.96681000e-03,
        1.60000000e+01,   6.96117000e-03, 1.64886600e+01,   7.22937600e-03, 1.64886800e+01,   7.22938700e-03, 1.65000000e+01,   7.23560000e-03,
        1.70000000e+01,   7.15708000e-03, 1.75000000e+01,   7.28194000e-03, 1.80000000e+01,   7.15160000e-03, 1.85000000e+01,   7.15239000e-03,
        1.90000000e+01,   6.98739000e-03, 1.94062600e+01,   6.92050300e-03, 1.94062700e+01,   6.92050200e-03, 1.95000000e+01,   6.90507000e-03,
        2.00000000e+01,   6.73953000e-03,
    };
    int nData = sizeof( data ) / ( 2 * sizeof( double ) );
    ptwXYPoints *crossSection;

    if( ( crossSection = ptwXY_create( smr, ptwXY_interpolationLinLin, NULL, 6, 1e-3, 10, 10, nData, data, 0 ) ) == NULL ) 
        nfut_printSMRErrorExit2p( smr, "Via." );
    return( crossSection);
}
/*
************************************************************
*/
static void writeXYDataOnVerbosity( ptwXYPoints *data, const char * const fileName ) {

    FILE *f;

    if( !verbose ) return;
    if( ( f = fopen( fileName, "w" ) ) == NULL ) nfu_printErrorMsg( "ERROR %s: could not open file %s\n", __FILE__, fileName );
    fprintf( f, "# length = %d\n", (int) ptwXY_length( NULL, data ) );
    ptwXY_simpleWrite( data, f, fmtXY );
    fclose( f );
}
/*
************************************************************
*/
static void writeXDataOnVerbosity( statusMessageReporting *smr, ptwXPoints *data, const char * const fileName ) {

    int64_t i;
    FILE *f;

    if( !verbose ) return;
    if( ( f = fopen( fileName, "w" ) ) == NULL ) nfu_printErrorMsg( "ERROR %s: could not open file %s\n", __FILE__, fileName );
    fprintf( f, "# length = %d\n", (int) ptwX_length( smr, data ) );
    for( i = 0; i < ptwX_length( smr, data ); i++ ) fprintf( f, fmtX, ptwX_getPointAtIndex_Unsafely( data, i ) );
    fclose( f );
}
