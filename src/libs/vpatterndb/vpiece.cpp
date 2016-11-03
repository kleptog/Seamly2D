/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vpiece.h"
#include "vpiece_p.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vabstractcurve.h"
#include "vcontainer.h"

#include <QSharedPointer>
#include <QDebug>

//---------------------------------------------------------------------------------------------------------------------
VPiece::VPiece()
    : VAbstractPiece(), d(new VPieceData)
{}

//---------------------------------------------------------------------------------------------------------------------
VPiece::VPiece(const VPiece &piece)
    : VAbstractPiece(piece), d (piece.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VPiece &VPiece::operator=(const VPiece &piece)
{
    if ( &piece == this )
    {
        return *this;
    }
    VAbstractPiece::operator=(piece);
    d = piece.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VPiece::~VPiece()
{}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief append append in the end of list node.
 * @param node new node.
 */
void VPiece::Append(const VPieceNode &node)
{
    d->nodes.append(node);
}

//---------------------------------------------------------------------------------------------------------------------
/** @brief Clear detail full clear. */
void VPiece::Clear()
{
    ClearNodes();
}

//---------------------------------------------------------------------------------------------------------------------
/** @brief ClearNodes clear list of nodes. */
void VPiece::ClearNodes()
{
    d->nodes.clear();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CountNode return count nodes.
 * @return count.
 */
qint32 VPiece::CountNode() const
{
    return d->nodes.size();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief operator [] find node by index in list.
 * @param indx index node in list.
 * @return node
 */
VPieceNode &VPiece::operator [](int indx)
{
    return d->nodes[indx];
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief at find node by index in list.
 * @param indx index node in list.
 * @return const node.
 */
const VPieceNode &VPiece::at(int indx) const
{
    return d->nodes.at(indx);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getNodes return list of nodes.
 * @return list of nodes.
 */
QVector<VPieceNode> VPiece::GetNodes() const
{
    return d->nodes;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setNodes set list of nodes
 * @param value list of nodes
 */
// cppcheck-suppress unusedFunction
void VPiece::SetNodes(const QVector<VPieceNode> &nodes)
{
    d->nodes = nodes;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPiece::MainPathPoints(const VContainer *data) const
{
    QVector<QPointF> points;
    for (int i = 0; i < CountNode(); ++i)
    {
        switch (at(i).GetTypeTool())
        {
            case (Tool::NodePoint):
            {
                const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(at(i).GetId());
                points.append(*point);
            }
            break;
            case (Tool::NodeArc):
            case (Tool::NodeSpline):
            case (Tool::NodeSplinePath):
            {
                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(at(i).GetId());

                const QPointF begin = StartSegment(data, i, at(i).GetReverse());
                const QPointF end = EndSegment(data, i, at(i).GetReverse());

                points << curve->GetSegmentPoints(begin, end, at(i).GetReverse());
            }
            break;
            default:
                qDebug()<<"Get wrong tool type. Ignore."<< static_cast<char>(at(i).GetTypeTool());
                break;
        }
    }

    points = CheckLoops(CorrectEquidistantPoints(points));//A path can contains loops
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VPiece::StartSegment(const VContainer *data, const int &i, bool reverse) const
{
    if (i < 0 && i > CountNode()-1)
    {
        return QPointF();
    }

    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(at(i).GetId());

    QVector<QPointF> points = curve->GetPoints();
    if (reverse)
    {
        points = VGObject::GetReversePoints(points);
    }

    QPointF begin = points.first();
    if (CountNode() > 1)
    {
        if (i == 0)
        {
            if (at(CountNode()-1).GetTypeTool() == Tool::NodePoint)
            {
                begin = *data->GeometricObject<VPointF>(at(CountNode()-1).GetId());
            }
        }
        else
        {
            if (at(i-1).GetTypeTool() == Tool::NodePoint)
            {
                begin = *data->GeometricObject<VPointF>(at(i-1).GetId());
            }
        }
    }
    return begin;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VPiece::EndSegment(const VContainer *data, const int &i, bool reverse) const
{
    if (i < 0 && i > CountNode()-1)
    {
        return QPointF();
    }

    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(at(i).GetId());

    QVector<QPointF> points = curve->GetPoints();
    if (reverse)
    {
        points = VGObject::GetReversePoints(points);
    }

    QPointF end = points.last();
    if (CountNode() > 2)
    {
        if (i == CountNode() - 1)
        {
            if (at(0).GetTypeTool() == Tool::NodePoint)
            {
                end = *data->GeometricObject<VPointF>(at(0).GetId());
            }
        }
        else
        {
            if (at(i+1).GetTypeTool() == Tool::NodePoint)
            {
                end = *data->GeometricObject<VPointF>(at(i+1).GetId());
            }
        }
    }
    return end;
}
