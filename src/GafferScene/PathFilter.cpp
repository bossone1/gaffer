//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "Gaffer/Context.h"

#include "GafferScene/ScenePlug.h"
#include "GafferScene/PathMatcherData.h"
#include "GafferScene/PathFilter.h"

using namespace GafferScene;
using namespace Gaffer;
using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PathFilter );

size_t PathFilter::g_firstPlugIndex = 0;

PathFilter::PathFilter( const std::string &name )
	:	Filter( name )
{
	storeIndexOfNextChild( g_firstPlugIndex );
	addChild( new StringVectorDataPlug( "paths", Plug::In, new StringVectorData ) );
	addChild( new ObjectPlug( "__pathMatcher", Plug::Out, new PathMatcherData ) );
}

PathFilter::~PathFilter()
{
}

Gaffer::StringVectorDataPlug *PathFilter::pathsPlug()
{
	return getChild<Gaffer::StringVectorDataPlug>( g_firstPlugIndex );
}

const Gaffer::StringVectorDataPlug *PathFilter::pathsPlug() const
{
	return getChild<Gaffer::StringVectorDataPlug>( g_firstPlugIndex );
}

Gaffer::ObjectPlug *PathFilter::pathMatcherPlug()
{
	return getChild<Gaffer::ObjectPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::ObjectPlug *PathFilter::pathMatcherPlug() const
{
	return getChild<Gaffer::ObjectPlug>( g_firstPlugIndex + 1 );
}

void PathFilter::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	Filter::affects( input, outputs );

	if( input == pathsPlug() )
	{
		outputs.push_back( pathMatcherPlug() );
	}
	else if( input == pathMatcherPlug() )
	{
		outputs.push_back( outPlug() );
	}
}

void PathFilter::hash( const Gaffer::ValuePlug *output, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	Filter::hash( output, context, h );

	if( output == pathMatcherPlug() )
	{
		pathsPlug()->hash( h );
	}
}

void PathFilter::compute( Gaffer::ValuePlug *output, const Gaffer::Context *context ) const
{
	if( output == pathMatcherPlug() )
	{
		ConstStringVectorDataPtr paths = pathsPlug()->getValue();
		PathMatcherDataPtr pathMatcherData = new PathMatcherData;
		pathMatcherData->writable().init( paths->readable().begin(), paths->readable().end() );
		static_cast<ObjectPlug *>( output )->setValue( pathMatcherData );
		return;
	}

	Filter::compute( output, context );
}

void PathFilter::hashMatch( const ScenePlug *scene, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	typedef IECore::TypedData<ScenePlug::ScenePath> ScenePathData;
	const ScenePathData *pathData = context->get<ScenePathData>( ScenePlug::scenePathContextName, 0 );
	if( pathData )
	{
		const ScenePlug::ScenePath &path = pathData->readable();
		h.append( &(path[0]), path.size() );
	}
	pathMatcherPlug()->hash( h );
}

unsigned PathFilter::computeMatch( const ScenePlug *scene, const Gaffer::Context *context ) const
{
	typedef IECore::TypedData<ScenePlug::ScenePath> ScenePathData;
	const ScenePathData *pathData = context->get<ScenePathData>( ScenePlug::scenePathContextName, 0 );
	if( pathData )
	{
		ConstPathMatcherDataPtr pathMatcher = boost::static_pointer_cast<const PathMatcherData>( pathMatcherPlug()->getValue() );
		return pathMatcher->readable().match( pathData->readable() );
	}
	return NoMatch;
}
