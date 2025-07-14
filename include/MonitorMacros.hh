#include "iss_sort.hh"
#include <iostream>

int PlotDiagnostics(){
	plot_diagnostic_hists();
	std::cout << "Plotting diagnostic histograms" << std::endl;
	return 0;
}

int ResetAll(){
	reset_conv_hists();
	reset_evnt_hists();
	reset_phys_hists();
	std::cout << "Reset ALL histograms" << std::endl;
	return 0;
}

int ResetConv(){
	reset_conv_hists();
	std::cout << "Reset singles histograms" << std::endl;
	return 0;
}

int ResetEvnt(){
	reset_evnt_hists();
	std::cout << "Reset event builder stage histograms" << std::endl;
	return 0;
}

int ResetHist(){
	reset_phys_hists();
	std::cout << "Reset physics stage histograms" << std::endl;
	return 0;
}

int StopMonitor(){
	stop_monitor();
	std::cout << "Stop monitoring" << std::endl;
	return 0;
}

int StartMonitor(){
	start_monitor();
	std::cout << "Start monitoring" << std::endl;
	return 0;
}

