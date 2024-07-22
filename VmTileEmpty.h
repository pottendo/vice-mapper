/*  -*-c++-*-
 * File:		VmTileEmpty.h
 * Date:		Mon Jul 27 15:17:50 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     empty tile class
 * 
 * Modifications:
 * $Log$
 */

#ifndef __VmTileEmpty_h__
#define __VmTileEmpty_h__

#include "VmTile.h"

class VmTileEmpty : public VmTile {
  protected:
    virtual void on_draw_specific(const Cairo::RefPtr<Cairo::Context>& cr) override;

  public:
    VmTileEmpty(VmMap &m, int x = -1, int y = -1);
    virtual ~VmTileEmpty() {};
};

#endif /* __VmTileEmpty_h__ */
