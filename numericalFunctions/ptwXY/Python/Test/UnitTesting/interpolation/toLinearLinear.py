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

import sys
sys.path.insert( 0, '../../../../../lib' )

import os
import pointwiseXY_C

options = ''
if( 'CHECKOPTIONS' in os.environ ) : options = os.environ['CHECKOPTIONS'].split( )

CPATH = '../../../../Test/UnitTesting/interpolation'

def skipBlankLines( ls ) :

    i = 0
    for i, l in enumerate( ls ) :
        if( l.strip( ) != '' ) : break
    ls = ls[i:]
    if( ( len( ls ) == 1 ) and ( ls[0].strip( ) == '' ) ) : ls = []
    return( ls )

def getIntegerValue( name, ls ) :

    s = "# %s = " % name
    n = len( s )
    if( ls[0][:n] != s ) : raise Exception( '%s: missing %s info: "%s"' % ( __file__, name, ls[0][:-1] ) )
    value = int( ls[0].split( '=' )[1] )
    return( ls[1:], value )

def getDoubleValue( name, ls ) :

    s = "# %s = " % name
    n = len( s )
    if( ls[0][:n] != s ) : raise Exception( '%s: missing %s info: "%s"' % ( __file__, name, ls[0][:-1] ) )
    value = float( ls[0].split( '=' )[1] )
    return( ls[1:], value )

def getXYData( ls, accuracy, interpolation = 'lin-lin' ) :

    ls, length = getIntegerValue( 'length', ls )
    data = [ map( float, ls[i].split( ) ) for i in xrange( length ) ]
    data = pointwiseXY_C.pointwiseXY_C( data, initialSize = len( data ), overflowSize = 10, accuracy = accuracy, interpolation = interpolation )
    ls = ls[length:]
    ls = skipBlankLines( ls )
    return( ls, data )

def checkIntepolation( xlog, ylog ) :

    xlogs = { 0 : 'lin', 1 : 'log' }
    ylogs = { 0 : 'lin', 1 : 'log' }
    f = open( os.path.join( CPATH, 'curve_sparse.dat' ) )
    ls = f.readlines( )
    f.close( )
    ls, xlog_ = getIntegerValue( 'xlog', ls )
    if( xlog != xlog_ ) : raise exception( 'xlog %s != xlog_ = %s' % ( xlog, xlog_ ) )
    ls, ylog_ = getIntegerValue( 'ylog', ls )
    if( ylog != ylog_ ) : raise exception( 'ylog %s != ylog_ = %s' % ( ylog, ylog_ ) )
    ls, accuracy = getDoubleValue( 'accuracy', ls )
    ls, sparseData = getXYData( ls, accuracy, interpolation = '%s-%s' % ( ylogs[ylog], xlogs[xlog] ) )

    f = open( os.path.join( CPATH, 'curve_dense.dat' ) )
    ls = f.readlines( )
    f.close( )
    ls, denseData = getXYData( ls, accuracy, interpolation = '%s-%s' % ( ylogs[ylog], xlogs[xlog] ) )

    f = open( os.path.join( CPATH, 'curve_linear.dat' ) )
    ls = f.readlines( )
    f.close( )
    ls, linearData = getXYData( ls, accuracy )

    n = sparseData.changeInterpolation( 'lin-lin', accuracy = accuracy )

    for x, y in denseData : 
        yn = n.evaluate( x )
        if( abs( y - yn ) > accuracy * ( abs( y ) + abs( yn ) ) ) : 
            raise Exception( '%s: at %s values %e and %e diff by %e' % ( __file__, x, y, yn , y - yn ) )

xlogs = { 0 : '', 1 : '-xlog' }
ylogs = { 0 : '', 1 : '-ylog' }
for xlog in xlogs :
    for ylog in ylogs :
        if( '-e' in options ) : print __file__, xlogs[xlog], ylogs[ylog]
        os.system( 'cd %s; make -s clean; ./toLinearLinear %s %s' % ( CPATH, xlogs[xlog], ylogs[ylog] ) )
        checkIntepolation( xlog, ylog )
