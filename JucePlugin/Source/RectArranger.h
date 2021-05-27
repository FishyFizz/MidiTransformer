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
    RectArranger(){ rect = juce::Rectangle<int>(0, 0, 0, 0); }
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
    juce::Array<RectArranger> EqualSplitHorizonal(int n)
    {
        if (n < 2) return { *this };
        juce::Array<RectArranger> result;
        int sizePerBlock = w() / n;
        RectArranger arealeft = *this;
        for (int i = 0; i < n - 1; i++)
        {
            auto p = arealeft.BisectL(sizePerBlock);
            result.add(p.first);
            arealeft = p.second;
        }
        result.add(arealeft);
        return result;
    }
    juce::Array<RectArranger> EqualSplitVertical(int n)
    {
        if (n < 2) return { *this };
        juce::Array<RectArranger> result;
        int sizePerBlock = h() / n;
        RectArranger arealeft = *this;
        for (int i = 0; i < n - 1; i++)
        {
            auto p = arealeft.BisectU(sizePerBlock);
            result.add(p.first);
            arealeft = p.second;
        }
        result.add(arealeft);
        return result;
    }
    int x() { return rect.getX(); }
    int y() { return rect.getY(); }
    int w() { return rect.getWidth(); }
    int h() { return rect.getHeight(); }
};