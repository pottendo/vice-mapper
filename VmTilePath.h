/*  -*-c++-*-
 * File:		VmTilePath.h
 * Date:		Mon Jul 27 16:50:58 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     path tile class
 * 
 * Modifications:
 * $Log$
 */

#ifndef __VmTilePath_h__
#define __VmTilePath_h__

#include "VmTile.h"

typedef enum { P_NW, P_N, P_NE, P_W, P_E, P_SW, P_S, P_SE } VmPathDir;

class VmTilePath : public VmTile {
    VmPathDir dir;
    double angle;
    
  protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    virtual void on_draw_specific(const Cairo::RefPtr<Cairo::Context>& cr) override;
    
  public:
    VmTilePath(VmMap &m, int x = -1, int y = -1, VmPathDir d = P_NW);
    VmTilePath(VmMap &m, VmPathDir dir);
    virtual ~VmTilePath() {};
};
\
#endif /* __VmTilePath_h__ */
