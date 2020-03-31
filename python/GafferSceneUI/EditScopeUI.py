##########################################################################
#
#  Copyright (c) 2019, Cinesite VFX Ltd. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import Gaffer
import GafferUI
import GafferScene
import GafferSceneUI

def addPruningActions( editor ) :

	if isinstance( editor, GafferUI.Viewer ) :
		editor.keyPressSignal().connect( __pruningKeyPress, scoped = False )

def __pruningKeyPress( viewer, event ) :

	if event.key not in ( "Backspace", "Delete" ) :
		return False

	if not isinstance( viewer.view(), GafferSceneUI.SceneView ) :
		return False

	editScope = viewer.view().editScope()
	if editScope is None or Gaffer.MetadataAlgo.readOnly( editScope ) :
		# We return True even when we don't do anything, so the keypress doesn't
		# leak out and get used to delete nodes in the node graph.
		## \todo Add a discreet notification system to the Viewer so we can
		# prompt the user to select a scope etc when necessary. Maybe we might
		# also want to ask them if we can prune a common ancestor in the case
		# that all its descendants are selected?
		return True

	viewedNode = viewer.view()["in"].getInput().node()
	if editScope != viewedNode and editScope not in Gaffer.NodeAlgo.upstreamNodes( viewedNode ) :
		# Spare folks from deleting things in a downstream EditScope.
		## \todo When we have a nice Viewer notification system we
		# should emit a warning here.
		return True

	sceneGadget = viewer.view().viewportGadget().getPrimaryChild()
	selection = sceneGadget.getSelection()
	if not selection.isEmpty() :
		with Gaffer.UndoScope( editScope.ancestor( Gaffer.ScriptNode ) ) :
			GafferScene.EditScopeAlgo.setPruned( editScope, selection, True )

	return True

