/****************************************************************************
** libmatroska : parse Matroska files, see http://www.matroska.org/
**
** <file/class description>
**
** Copyright (C) 2002-2010 Steve Lhomme.  All rights reserved.
**
** This file is part of libmatroska.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** See http://www.matroska.org/license/lgpl/ for LGPL licensing information.**
** Contact license@matroska.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
	\file
	\version \$Id: KaxSeekHead.cpp 640 2004-07-09 21:05:36Z mosu $
	\author Steve Lhomme     <robux4 @ users.sf.net>
*/
#include "matroska/KaxSeekHead.h"
#include "matroska/KaxContexts.h"
#include "matroska/KaxSegment.h"
#include "matroska/KaxCues.h"
#include "matroska/KaxDefines.h"

using namespace LIBEBML_NAMESPACE;

// sub elements
START_LIBMATROSKA_NAMESPACE

static const EbmlSemantic ContextList_KaxSeekHead[1] =
{
	EbmlSemantic(true,  false,  EBML_INFO(KaxSeek)),
};

static const EbmlSemantic ContextList_KaxSeek[2] = 
{
	EbmlSemantic(true,  true,  EBML_INFO(KaxSeekID)),
	EbmlSemantic(true,  true,  EBML_INFO(KaxSeekPosition)),
};

DEFINE_MKX_MASTER  (KaxSeekHead, 0x114D9B74, 4, KaxSegment, "SeekHeader");
DEFINE_MKX_MASTER  (KaxSeek,         0x4DBB, 2, KaxSeekHead, "SeekPoint");
DEFINE_MKX_BINARY  (KaxSeekID,       0x53AB, 2, KaxSeek, "SeekID");
DEFINE_MKX_UINTEGER(KaxSeekPosition, 0x53AC, 2, KaxSeek, "SeekPosition");

KaxSeekHead::KaxSeekHead()
	:EbmlMaster(KaxSeekHead_Context)
{}

KaxSeek::KaxSeek()
	:EbmlMaster(KaxSeek_Context)
{}

/*!
	\todo verify that the element is not already in the list
*/
void KaxSeekHead::IndexThis(const EbmlElement & aElt, const KaxSegment & ParentSegment)
{
	// create a new point
	KaxSeek & aNewPoint = AddNewChild<KaxSeek>(*this);

	// add the informations to this element
	KaxSeekPosition & aNewPos = GetChild<KaxSeekPosition>(aNewPoint);
	*static_cast<EbmlUInteger *>(&aNewPos) = ParentSegment.GetRelativePosition(aElt);

	KaxSeekID & aNewID = GetChild<KaxSeekID>(aNewPoint);
	binary ID[4];
	for (int i=EBML_ID_LENGTH(EbmlId(aElt)); i>0; i--) {
		ID[4-i] = (EBML_ID_VALUE(EbmlId(aElt)) >> 8*(i-1)) & 0xFF;
	}
	aNewID.CopyBuffer(ID, EBML_ID_LENGTH(EbmlId(aElt)));
}

KaxSeek * KaxSeekHead::FindFirstOf(const EbmlCallbacks & Callbacks) const
{
	// parse all the Entries and find the first to match the type
	KaxSeek * aElt = static_cast<KaxSeek *>(FindFirstElt(EBML_INFO(KaxSeek)));
	while (aElt != NULL)
	{
		KaxSeekID * aId = NULL;
		for (unsigned int i = 0; i<aElt->ListSize(); i++) {
			if (EbmlId(*(*aElt)[i]) == EBML_ID(KaxSeekID)) {
				aId = static_cast<KaxSeekID*>((*aElt)[i]);
				EbmlId aEbmlId(aId->GetBuffer(), aId->GetSize());
				if (aEbmlId == EBML_INFO_ID(Callbacks))
				{
					return aElt;
				}
				break;
			}
		}
		aElt = static_cast<KaxSeek *>(FindNextElt(*aElt));
	}

	return NULL;
}

KaxSeek * KaxSeekHead::FindNextOf(const KaxSeek &aPrev) const
{
	unsigned int iIndex;
	KaxSeek *tmp;
	
	// look for the previous in the list
	for (iIndex = 0; iIndex<ListSize(); iIndex++)
	{
		if ((*this)[iIndex] == static_cast<const EbmlElement*>(&aPrev))
			break;
	}

	if (iIndex <ListSize()) {
		iIndex++;
		for (; iIndex<ListSize(); iIndex++)
		{
			if (EbmlId(*((*this)[iIndex])) == EBML_ID(KaxSeek))
			{
				tmp = (KaxSeek *)((*this)[iIndex]);
				if (tmp->IsEbmlId(aPrev))
					return tmp;
			}
		}
	}

	return NULL;
}

int64 KaxSeek::Location() const
{
	KaxSeekPosition *aPos = static_cast<KaxSeekPosition*>(FindFirstElt(EBML_INFO(KaxSeekPosition)));
	if (aPos == NULL)
		return 0;
	return uint64(*aPos);
}

bool KaxSeek::IsEbmlId(const EbmlId & aId) const
{
	KaxSeekID *_Id = static_cast<KaxSeekID*>(FindFirstElt(EBML_INFO(KaxSeekID)));
	if (_Id == NULL)
		return false;
	EbmlId aEbmlId(_Id->GetBuffer(), _Id->GetSize());
	return (aId == aEbmlId);
}

bool KaxSeek::IsEbmlId(const KaxSeek & aPoint) const
{
	KaxSeekID *_IdA = static_cast<KaxSeekID*>(FindFirstElt(EBML_INFO(KaxSeekID)));
	if (_IdA == NULL)
		return false;
	KaxSeekID *_IdB = static_cast<KaxSeekID*>(aPoint.FindFirstElt(EBML_INFO(KaxSeekID)));
	if (_IdB == NULL)
		return false;
	EbmlId aEbmlIdA(_IdA->GetBuffer(), _IdA->GetSize());
	EbmlId aEbmlIdB(_IdB->GetBuffer(), _IdB->GetSize());
	return (aEbmlIdA == aEbmlIdB);
}

END_LIBMATROSKA_NAMESPACE
