#include "MyStyle.hh"
#include <TStyle.h>

TStyle* SetStyle(){
	
	TStyle* Style = new TStyle("MyStyle","Marcello style");
	
	// Canvas colors, borders.
	Style->SetCanvasBorderMode(0);
	Style->SetPadBorderMode(0);
	Style->SetPadBorderSize(0);
	Style->SetPadColor(0);
	Style->SetCanvasColor(0);
	Style->SetTitleFillColor(0);
	Style->SetTitleBorderSize(0);
	Style->SetStatColor(0);
	Style->SetStatBorderSize(0);
	Style->SetLegendFillColor(0);
	Style->SetLegendBorderSize(0);
	Style->SetFrameBorderMode(0);
	
	// Paper and margin size
	Style->SetPaperSize(20,26);
	Style->SetPadTopMargin(0.08);
	Style->SetPadRightMargin(0.08);
	Style->SetPadBottomMargin(0.16);
	Style->SetPadLeftMargin(0.12);
	
	// Set axis grid.
	Style->SetGridStyle(5);
	Style->SetGridWidth(0.1);
	//Style->SetNdivisions(510);
	
	// Set Stat box position.
	Style->SetStatX(0.9);
	Style->SetStatY(0.8);
	
	// Set tile position.
	Style->SetTitleAlign(13);
	Style->SetTitleX(0.15);
	Style->SetTitleY(1.0);
	
	// Statistic box by default.
	Style->SetOptStat("mreuo");
	// Style->SetOptStat(0);
	
	int font =10*6 + 2; //10*font code + precision
	double fontSize=0.04;
	
	Style->SetTitleFont(font,"xyz");
	Style->SetTitleSize(fontSize,"xyz");
	Style->SetTitleOffset(1.5,"xyz");
	Style->SetLabelFont(font,"xyz");
	Style->SetLabelSize(fontSize,"xyz");
	
	// Set palette.
	Style->SetPalette(1,0);
	
	return Style;
	
}

