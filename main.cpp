#include <corsika/Index.h>
#include <corsika/Units.h>
#include <corsika/ShowerFile.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;
using namespace corsika;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <CORSIKA DAT file>" << endl;
        return 1;
    }

    // Open the CORSIKA file
    ShowerFile file(argv[1]);
    if (!file.IsOpen()) {
        cerr << "Failed to open file: " << argv[1] << endl;
        return 1;
    }

    cout << "Opening " << argv[1] << endl;

    // Variables to determine dynamic histogram ranges
    double minEnergy = numeric_limits<double>::max();
    double maxEnergy = numeric_limits<double>::lowest();
    double minX = numeric_limits<double>::max();
    double maxX = numeric_limits<double>::lowest();
    double minY = numeric_limits<double>::max();
    double maxY = numeric_limits<double>::lowest();

    int totalParticles = 0;
    int muonCount = 0;
    double muonEnergySum = 0;
    int eventNumber = 1;

    // Determine the ranges for energy and positions
    while (file.FindEvent(eventNumber) == eSuccess) {
        auto particles = file.GetCurrentShower().ParticleStream();
        while (auto it = particles.NextParticle()) {
            int p_type = it->PDGCode();
            // Only process muons (PDG: 13 = μ⁻, -13 = μ⁺)
            if (p_type == 13 || p_type == -13) {
                double energy = it->KineticEnergy() / GeV;
                minEnergy = std::min(minEnergy, energy);
                maxEnergy = std::max(maxEnergy, energy);
                minX = std::min(minX, static_cast<double>(it->fX));
                maxX = std::max(maxX, static_cast<double>(it->fX));
                minY = std::min(minY, static_cast<double>(it->fY));
                maxY = std::max(maxY, static_cast<double>(it->fY));
            }
        }
        eventNumber++;
    }

    // Adjust ranges if no muons were found
    if (minEnergy == numeric_limits<double>::max()) minEnergy = 0;
    if (maxEnergy == numeric_limits<double>::lowest()) maxEnergy = 1;
    if (minX == numeric_limits<double>::max()) minX = -1000;
    if (maxX == numeric_limits<double>::lowest()) maxX = 1000;
    if (minY == numeric_limits<double>::max()) minY = -1000;
    if (maxY == numeric_limits<double>::lowest()) maxY = 1000;

    // Create histograms with determined ranges
    TH1F* hMuonEnergy = new TH1F("hMuonEnergy", "Muon Kinetic Energy Distribution; Energy (GeV); Counts", 100, minEnergy, maxEnergy);
    hMuonEnergy->SetDirectory(0);  // Prevent automatic deletion

    TH2F* hMuonPosition = new TH2F("hMuonPosition", "Muon Position Distribution; X (cm); Y (cm)", 100, minX, maxX, 100, minY, maxY);
    hMuonPosition->SetDirectory(0);  // Prevent automatic deletion

    // Reset event counter
    eventNumber = 1;

    // Fill the histograms
    while (file.FindEvent(eventNumber) == eSuccess) {
        auto particles = file.GetCurrentShower().ParticleStream();
        while (auto it = particles.NextParticle()) {
            int p_type = it->PDGCode();
            if (p_type == 13 || p_type == -13) {
                double energy = it->KineticEnergy() / GeV;
                hMuonEnergy->Fill(energy);
                hMuonPosition->Fill(static_cast<double>(it->fX), static_cast<double>(it->fY));
                muonEnergySum += energy;
                muonCount++;
            }
            totalParticles++;
        }
        eventNumber++;
    }

    cout << "Total Particles: " << totalParticles << endl;
    cout << "Muons: " << muonCount << endl;
    cout << "Total Muon Energy: " << muonEnergySum << " GeV" << endl;

    // Output histogram entries
    cout << "Muon Energy Histogram Entries: " << hMuonEnergy->GetEntries() << endl;
    cout << "Muon Position Histogram Entries: " << hMuonPosition->GetEntries() << endl;

    // Create a canvas and draw histograms
    TCanvas* c1 = new TCanvas("c1", "Muon Data", 1200, 600);
    c1->Divide(2, 1);

    c1->cd(1);
    gPad->SetLogy();  // Enable log scale for energy distribution
    hMuonEnergy->Draw("hist");

    c1->cd(2);
    hMuonPosition->Draw("colz");

    c1->Modified();
    c1->Update();
    c1->SaveAs("shower_plot_GeV.png");

    // Save histograms to a ROOT file
    TFile outFile("shower_data_GeV.root", "RECREATE");
    hMuonEnergy->Write();
    hMuonPosition->Write();
    outFile.Close();

    cout << "Plots saved as 'shower_plot_GeV.png' and 'shower_data_GeV.root'" << endl;

    // Clean up
    delete c1;
    delete hMuonEnergy;
    delete hMuonPosition;

    return 0;
}