/**
 * IVSequencerControl.h by Tim Fan <fanchenhao2206@icloud.com>
 * 
 * A simple step sequencer whose number of rows and columns are passed 
 * as template parameters, and is edited using mouse clicks and drags; 
 * TODO: on each edit, send MIDI message to DSP to audition new note;
 * this should be done through the provided updateFn, i think is plan
 */

#pragma once

#include "IControl.h"
#include <vector>

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

class IVSequencerControl : public IControl
{
public:
  IVSequencerControl(const IRECT& bounds, int nRows, int nCols,
    std::function<void()> updateFn = nullptr)
  : IControl(bounds, kNoParameter), mUpdateFn(updateFn),
    mClickCol(-1), mClickRow(-1), mHoverCol(-1), mHoverRow(-1),
    mNCols(nCols), // Each column represents a step; e.g. 16 columns for a 16-step sequencer
    mNRows(nRows), // Each row represents the value for the corresponding step; e.g. play E4 on this step
    mCells(nCols, -1)
  {}

  void Draw(IGraphics& g) override
  {
    g.FillRect(COLOR_WHITE, mRECT);
    g.DrawRect(COLOR_BLACK, mRECT, 0, 4.f);

    // sketch:
    char str[64];
    sprintf(str, "col: %d, row: %d", mHoverCol, mHoverRow);
    g.DrawText(DEFAULT_TEXT, str, mRECT);

    // Draw grid: nRow by nCol cells
    float rowSpacing = mRECT.H() / mNRows;
    float colSpacing = mRECT.W() / mNCols;

    // Fill in hover over cell
    g.FillRect(COLOR_GRAY, IRECT::MakeXYWH(
      mRECT.L + (mHoverCol * colSpacing), mRECT.T + (mHoverRow * rowSpacing),
      colSpacing, rowSpacing));

    // Fill in selected cells
    assert(mCells.size() == mNCols);
    for (int i = 0; i < mNCols; i++) {
      int cell = mCells[i];
      if (cell < 0) continue;

      // Fill in the cell at column i, and row given by cell
      g.FillRect(COLOR_BLACK, IRECT::MakeXYWH(
        mRECT.L + (i * colSpacing), mRECT.T + (cell * rowSpacing),
        colSpacing, rowSpacing));
    }

    // Draw dividing lines
    for (int i = 1; i <= mNRows-1; i++) {
      // Draw nRow-1 horizontal lines
      float y = mRECT.T + (i * rowSpacing);

      // Make it easier to track which note
      float thickness = (i % 4 == 0) ? 4.f : 1.f; 
      g.DrawLine(COLOR_BLACK, mRECT.L, y, mRECT.R, y, 0, thickness);
    }
    for (int i = 1; i <= mNCols-1; i++) {
      // Draw nCol-1 vertical lines
      float x = mRECT.L + (i * colSpacing);

      // Emphasize the bar lines
      float thickness = (i % 4 == 0) ? 4.f : 1.f; 
      g.DrawLine(COLOR_BLACK, x, mRECT.B, x, mRECT.T, 0, thickness);
    }
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override
  {
    // Mouse needs to be in-bounds to edit the sequencer
    if (!mRECT.Contains(x, y)) return;

    // (x,y) are given as absolute coordinates; now that we know it 
    // is a point within mRECT, let's find its relative coordinates
    float relX = x - mRECT.L;
    float relY = y - mRECT.T;

    // Then, it's easy to find which grid this point belongs in
    float rowSpacing = mRECT.H() / mNRows;
    float colSpacing = mRECT.W() / mNCols;

    float colIdx = (int)(relX / colSpacing);
    float rowIdx = (int)(relY / rowSpacing);

    // So, turn it on! Or, off it is already on
    if (mCells[colIdx] == rowIdx) {
      // We clicked on (row,col), but that is already on; so turn off
      mCells[colIdx] = -1;
    } else {
      // We clicked on (row,col), which is not already on, so turn it on
      mCells[colIdx] = rowIdx;
    }

    // And re-draw control
    SetDirty(true);

    // Used by OnMouseDrag to know what mode it is in
    mClickCol = colIdx;
    mClickRow = rowIdx;
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override
  {
    // Mouse needs to be in-bounds to edit the sequencer
    if (!mRECT.Contains(x, y)) return;

    // (x,y) are given as absolute coordinates; now that we know it 
    // is a point within mRECT, let's find its relative coordinates
    float relX = x - mRECT.L;
    float relY = y - mRECT.T;

    // Then, it's easy to find which grid this point belongs in
    float rowSpacing = mRECT.H() / mNRows;
    float colSpacing = mRECT.W() / mNCols;

    float colIdx = (int)(relX / colSpacing);
    float rowIdx = (int)(relY / rowSpacing);

    // So, turn it on! Or, off, depending on mClickCol/Row
    if (mCells[mClickCol] > -1) {
      // User turned ON the cell they clicked, so lets turn ON this cell too
      mCells[colIdx] = rowIdx;
    } else {
      // User turned OFF the cell they clicked, so lets turn OFF this cell too
      if (mCells[colIdx] == rowIdx) {
        // As long as it was previously on...
        mCells[colIdx] = -1;
      }
    }

    mHoverCol = colIdx;
    mHoverRow = rowIdx;

    // And re-draw control
    SetDirty(true);
  }

  void OnMouseOver(float x, float y, const IMouseMod& mod) override
  {
    // Mouse needs to be in-bounds to edit the sequencer
    if (!mRECT.Contains(x, y)) return;

    // (x,y) are given as absolute coordinates; now that we know it 
    // is a point within mRECT, let's find its relative coordinates
    float relX = x - mRECT.L;
    float relY = y - mRECT.T;

    // Then, it's easy to find which grid this point belongs in
    float rowSpacing = mRECT.H() / mNRows;
    float colSpacing = mRECT.W() / mNCols;

    float colIdx = (int)(relX / colSpacing);
    float rowIdx = (int)(relY / rowSpacing);

    mHoverCol = colIdx;
    mHoverRow = rowIdx;

    // And re-draw control
    SetDirty(true);
  }

  void OnMouseOut() override
  {
    mHoverCol = -1;
    mHoverRow = -1;

    // And re-draw control
    SetDirty(true);
  }

  /* .cpp files that extend me (e.g. IMidiSequencer) can override this */
  virtual void OnNewValue()
  {
    if(mUpdateFn)
      mUpdateFn();
  }
  
protected:
  std::function<void()> mUpdateFn;
private:
  int mClickCol, mClickRow;
  int mHoverCol, mHoverRow;
public:
  std::vector<int> mCells;
  int mNCols, mNRows;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
