#include "Image.h"
#include "SeamCarver.h"

#include <algorithm>
#include <fstream>
#include <iostream>

std::vector<std::vector<Image::Pixel>> ReadImageFromCSV(std::ifstream & input)
{
    size_t width, height;
    input >> width >> height;
    std::vector<std::vector<Image::Pixel>> table;
    for (size_t columnId = 0; columnId < width; ++columnId) {
        std::vector<Image::Pixel> column;
        for (size_t rowId = 0; rowId < height; ++rowId) {
            size_t red, green, blue;
            input >> red >> green >> blue;
            column.emplace_back(red, green, blue);
        }
        table.emplace_back(std::move(column));
    }
    return table;
}

void WriteImageToCSV(const SeamCarver & carver, std::ofstream & output)
{
    const size_t width = carver.GetImageWidth();
    const size_t height = carver.GetImageHeight();
    output << width << " " << height << "\n";
    const Image & image = carver.GetImage();
    for (size_t columnId = 0; columnId < width; ++columnId) {
        for (size_t rowId = 0; rowId < height; ++rowId) {
            const Image::Pixel & pixel = image.GetPixel(columnId, rowId);
            output << pixel.m_red << " " << pixel.m_green << " " << pixel.m_blue << std::endl;
        }
    }
}

int main(int argc, char * argv[])
{
    //something like a regression test
    // (mistake in previous version was caused by initializing res as int instead of double)
    // here is a part of algorithm, which is identical to the algorithm I use in SeamCarver to find a vertical seam
    std::vector<std::vector<double>> GetPixelEnergy = {{167, 140.64494303031304, 270, 214, 185, 143.89232085139221, 173}, {119.17214439624723, 205, 165, 165, 251, 115.21284650593439, 173}};
    std::vector<std::vector<double>> energies;
    std::vector<std::vector<std::vector<size_t>>> seams;
    double GetImageHeight = 2;
    double GetImageWidth = 7;
    for (size_t row = 0; row < GetImageHeight; row++) {
        energies.push_back(std::vector<double>());
        seams.push_back(std::vector<std::vector<size_t>>());
        for (size_t col = 0; col < GetImageWidth; col++) {
            energies[row].push_back(0.0);
            seams[row].push_back(std::vector<size_t>());
        }
    }
    for (size_t col = 0; col < GetImageWidth; col++) {
        energies[0][col] = GetPixelEnergy[0][col];
        seams[0][col].push_back(col);
    }
    for (size_t row = 1; row < GetImageHeight; row++) {
        for (size_t col = 0; col < GetImageWidth; col++) {
            double temp = energies[row - 1][col];
            std::vector tempseam = seams[row - 1][col];
            if (col > 0 && energies[row - 1][col - 1] < temp) {
                temp = energies[row - 1][col - 1];
                tempseam = seams[row - 1][col - 1];
            }
            if (col < GetImageWidth - 1 && energies[row - 1][col + 1] < temp) {
                temp = energies[row - 1][col + 1];
                tempseam = seams[row - 1][col + 1];
            }
            energies[row][col] = temp + GetPixelEnergy[row][col];
            seams[row][col] = tempseam;
            seams[row][col].push_back(col);
        }
    }
    std::cerr << "Energy map:" << std::endl;
    for (const auto & i : energies) {
        for (const auto j : i) {
            std::cerr << j << " ";
        }
        std::cerr << std::endl;
    }
    double res = std::min_element(energies[GetImageHeight - 1].begin(), energies[GetImageHeight - 1].end()) - energies[GetImageHeight - 1].begin();
    std::vector<size_t> resseam = seams[GetImageHeight - 1][res];
    std::cerr << "seam: ";
    for (const auto i : resseam) {
        std::cerr << i << " ";
    }
    std::cerr << std::endl;
    //the end of regression test, start of main

    // Check command line arguments
    const size_t expectedAmountOfArgs = 3;
    if (argc != expectedAmountOfArgs) {
        std::cout << "Wrong amount of arguments. Provide filenames as arguments. See example below:\n";
        std::cout << "seam-carving data/tower.csv data/tower_updated.csv" << std::endl;
        return 0;
    }
    // Check csv file
    std::ifstream inputFile(argv[1]);
    if (!inputFile.good()) {
        std::cout << "Can't open source file " << argv[1] << ". Verify that the file exists." << std::endl;
    }
    else {
        auto imageSource = ReadImageFromCSV(inputFile);
        SeamCarver carver(std::move(imageSource));
        std::cout << "Image: " << carver.GetImageWidth() << "x" << carver.GetImageHeight() << std::endl;
        const size_t pixelsToDelete = 150;
        for (size_t i = 0; i < pixelsToDelete; ++i) {
            std::vector<size_t> seam = carver.FindVerticalSeam();
            carver.RemoveVerticalSeam(seam);
            std::cout << "width = " << carver.GetImageWidth() << ", height = " << carver.GetImageHeight() << std::endl;
        }
        std::ofstream outputFile(argv[2]);
        WriteImageToCSV(carver, outputFile);
        std::cout << "Updated image is written to " << argv[2] << "." << std::endl;
    }
    return 0;
}