/*
  ==============================================================================

    RectArranger.h
    Created: 27 May 2021 12:29:46am
    Author:  Fizz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <map>
using namespace std;

class RectArranger
{
public:
    juce::Rectangle<int> rect;
    RectArranger(juce::Rectangle<int>& const r) { rect = r; }
    RectArranger(int x, int y, int w, int h) { rect = juce::Rectangle<int>(x, y, w, h); }
    operator juce::Rectangle<int>() { return rect; }

    pair<RectArranger, RectArranger> BisectU(int pos)
    {
        return make_pair(
            RectArranger(rect.getX(), rect.getY(), rect.getWidth(), pos),
            RectArranger(rect.getX(), rect.getY() + pos+1, rect.getWidth(), rect.getHeight() - pos-1));
    }
    pair<RectArranger, RectArranger> BisectD(int pos)
    {
        return BisectU(rect.getHeight() - pos);
    }
    pair<RectArranger, RectArranger> BisectL(int pos)
    {
        return make_pair(
            RectArranger(rect.getX(), rect.getY(), pos, rect.getHeight()),
            RectArranger(rect.getX()+pos+1, rect.getY(), rect.getWidth()-pos-1, rect.getHeight()));
    }
    pair<RectArranger, RectArranger> BisectR(int pos)
    {
        return BisectL(rect.getWidth() - pos);
    }
    int x() { return rect.getX(); }
    int y() { return rect.getY(); }
    int w() { return rect.getWidth(); }
    int h() { return rect.getHeight(); }
};