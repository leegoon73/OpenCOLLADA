/*
Copyright (c) 2008 NetAllied Systems GmbH

This file is part of COLLADAMax.

Portions of the code are:
Copyright (c) 2005-2007 Feeling Software Inc.
Copyright (c) 2005-2007 Sony Computer Entertainment America

Based on the 3dsMax COLLADASW Tools:
Copyright (c) 2005-2006 Autodesk Media Entertainment

Licensed under the MIT Open Source License, 
for details please see LICENSE file or the website
http://www.opensource.org/licenses/mit-license.php
*/

#include "COLLADAMaxStableHeaders.h"
#include "COLLADAMaxNodeImporter.h"

#include "dummy.h"

namespace COLLADAMax
{

	NodeImporter::NodeImporter( DocumentImporter* documentImporter )
		:	ImporterBase(documentImporter)
	{}

	//------------------------------
	NodeImporter::~NodeImporter()
	{
	}


	//---------------------------------------------------------------
	void NodeImporter::Matrix4ToMaxMatrix3 ( Matrix3 & copy,  const COLLADABU::Math::Matrix4& original )
	{
		Point4 column;
		column[ 0 ] = (float)original.getElement(0,0);
		column[ 1 ] = (float)original.getElement(0,1);
		column[ 2 ] = (float)original.getElement(0,2);
		column[ 3 ] = (float)original.getElement(0,3);
		copy.SetColumn(0, column);

		column[ 0 ] = (float)original.getElement(1,0);
		column[ 1 ] = (float)original.getElement(1,1);
		column[ 2 ] = (float)original.getElement(1,2);
		column[ 3 ] = (float)original.getElement(1,3);
		copy.SetColumn(1, column);

		column[ 0 ] = (float)original.getElement(2,0);
		column[ 1 ] = (float)original.getElement(2,1);
		column[ 2 ] = (float)original.getElement(2,2);
		column[ 3 ] = (float)original.getElement(2,3);
		copy.SetColumn(2, column);
	}

	//------------------------------
	void NodeImporter::setNodeProperties( const COLLADAFW::Node* node, ImpNode* importNode)
	{
		String newNodeName = node->getName();
		if ( !newNodeName.empty() )
			importNode->SetName(newNodeName.c_str());

		// set transform
		COLLADABU::Math::Matrix4 transformationMatrix;
		node->getTransformationMatrix(transformationMatrix);
		Matrix3 maxTransformationMatrix;
		Matrix4ToMaxMatrix3(maxTransformationMatrix, transformationMatrix);
		importNode->SetTransform(0, maxTransformationMatrix);

	}

	//------------------------------
	ImpNode* NodeImporter::importNode( const COLLADAFW::Node* node, ImpNode* parentImportNode )
	{
		bool singleGeometryInstance = node->getInstanceGeometries().getCount() == 1;
		ImpNode* newImportNode = 0;

		if ( !singleGeometryInstance )
		{
			newImportNode = getMaxImportInterface()->CreateNode();

			setNodeProperties(node, newImportNode);
			getMaxImportInterface()->AddNodeToScene(newImportNode);

			RefResult res = newImportNode->Reference(getDummyObject());

			importInstanceGeometries(node->getInstanceGeometries(), newImportNode);
			importNodes(node->getChildNodes(), newImportNode);
		}
		else
		{
			newImportNode = importInstanceGeometrie( node, parentImportNode );
			importNodes(node->getChildNodes(), parentImportNode);
		}

		// Append all nodes that are referenced by this node.
		importInstanceNodes(node->getInstanceNodes(), newImportNode);

		/** Store the unique id of the created node, to resolve references, when ever necessary.*/ 
		addUniqueIdINodePair(node->getUniqueId(), newImportNode->GetINode());

		INode* childNode = newImportNode->GetINode();
		INode* parentNode = parentImportNode->GetINode();
		parentNode->AttachChild(childNode, FALSE);

		/* if there are nodes that reference the just created node, clone this node
		and append it to the referencing node.*/
		ImpNodeList referencingImpNodeList;
		getReferencingImpNodesByUniqueId(node->getUniqueId(), referencingImpNodeList);
		for ( size_t i = 0, count = referencingImpNodeList.size(); i<count; ++i)
			recursivlyCloneINode( referencingImpNodeList[i], newImportNode->GetINode() );


		return newImportNode;
	}

	//------------------------------
	bool NodeImporter::importNodes( const COLLADAFW::NodePointerArray& nodeArray, ImpNode* parentImportNode )
	{
		for ( size_t i = 0, count = nodeArray.getCount(); i < count; ++i)
		{
			ImpNode* newNode = importNode(nodeArray[i], parentImportNode);
		}

		return true;
	}

	//------------------------------
	bool NodeImporter::importInstanceGeometries( const COLLADAFW::InstanceGeometryPointerArray& instanceGeometryArray, ImpNode* parentImportNode )
	{
		for ( size_t i = 0, count = instanceGeometryArray.getCount(); i < count; ++i)
		{
			COLLADAFW::InstanceGeometry* instanceGeometry = instanceGeometryArray[i];

			ImpNode* newImportNode = getMaxImportInterface()->CreateNode();
			INode* newNode = newImportNode->GetINode();
			const COLLADAFW::UniqueId& uniqueId = instanceGeometry->getInstanciatedObjectId();

			Object* object = getObjectByUniqueId(uniqueId);
			if ( object )
			{
				newImportNode->Reference(object);
			}
			else
			{
				newImportNode->Reference( getDummyObject() );
			}
			const COLLADAFW::UniqueId& instanceGeometryUniqueId = instanceGeometry->getInstanciatedObjectId();
			// Store mapping between unique ids and nodes referencing the corresponding object.
			// Used to clone nodes
			addObjectINodeUniqueIdPair(newNode, instanceGeometryUniqueId);
			// Used to resolve instancing of objects
			addUniqueIdObjectINodePair(instanceGeometryUniqueId, newNode);

			INode* parentNode = parentImportNode->GetINode();
			parentNode->AttachChild(newNode, FALSE);
		}

		return true;
	}

	//------------------------------
	ImpNode* NodeImporter::importInstanceGeometrie( const COLLADAFW::Node* node, ImpNode* parentImportNode )
	{
		ImpNode* newImportNode = getMaxImportInterface()->CreateNode();
		setNodeProperties(node, newImportNode);
		INode* newNode = newImportNode->GetINode();

		COLLADAFW::InstanceGeometry* instanceGeometry = node->getInstanceGeometries()[0];
		const COLLADAFW::UniqueId& uniqueId = instanceGeometry->getInstanciatedObjectId();

		Object* object = getObjectByUniqueId(uniqueId);
		if ( object )
		{
			newImportNode->Reference(object);
		}
		else
		{
			newImportNode->Reference( getDummyObject() );
		}

		const COLLADAFW::UniqueId& instanceGeometryUniqueId = instanceGeometry->getInstanciatedObjectId();
		// Store mapping between unique ids and nodes referencing the coresponing object.
		// Used to clone nodes
		addObjectINodeUniqueIdPair(newNode, instanceGeometryUniqueId);
		// Used to resolve instancing of objects
		addUniqueIdObjectINodePair(instanceGeometryUniqueId, newNode);
		INode* parentNode = parentImportNode->GetINode();
		parentNode->AttachChild(newNode, FALSE);
		return newImportNode;
	}


	//------------------------------
	bool NodeImporter::importInstanceNodes( const COLLADAFW::InstanceNodePointerArray& instanceNodeArray, ImpNode* parentImportNode )
	{
		for ( size_t i = 0, count = instanceNodeArray.getCount(); i < count; ++i)
		{
			COLLADAFW::InstanceNode* instanceNode = instanceNodeArray[i];

			const COLLADAFW::UniqueId& uniqueId = instanceNode->getInstanciatedObjectId();

			INode* instanciatedINode = getINodeByUniqueId(uniqueId);
			if ( instanciatedINode )
			{
				if ( !recursivlyCloneINode(parentImportNode, instanciatedINode) )
					return false;;
			}
			else
			{
				// If the referenced node has not been imported, store which node is referenced
				// to clone the nodes as the referenced nodes gets imported
				addUniqueIdReferencingImpNodePair(instanceNode->getInstanciatedObjectId(), parentImportNode );
			}
		}

		return true;
	}

	//------------------------------
	bool NodeImporter::recursivlyCloneINode( ImpNode* parentImportNode, INode* nodeToClone )
	{
		ImpNode* newImportNode = getMaxImportInterface()->CreateNode();
		getMaxImportInterface()->AddNodeToScene(newImportNode);

		INode* newNode = newImportNode->GetINode();

		Object* object = nodeToClone->GetObjectRef();
		newImportNode->Reference(object);
		newNode->SetTMController(nodeToClone->GetTMController());
		newImportNode->SetName(nodeToClone->GetName());

		INode* parentNode = parentImportNode->GetINode();
		parentNode->AttachChild(newNode, TRUE);

		/* If the node to clone references an object, the cloned one must references the same object.*/
		COLLADAFW::UniqueId id = getUniqueIdByObjectINode(nodeToClone);
		if ( id.isValid() )
			addUniqueIdObjectINodePair(id, newNode);

		// Clone the children 
		for ( int i = 0, count = nodeToClone->NumberOfChildren(); i < count; ++i)
			recursivlyCloneINode(newImportNode, nodeToClone->GetChildNode(i));

		return true;
	}



} // namespace COLLADAMax