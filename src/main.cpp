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

void regressiontest()
{
    const Image test({{{186, 73, 178}, {50, 93, 116}, {252, 102, 248}}, {{237, 102, 135}, {64, 174, 101}, {247, 9, 24}}, {{153, 109, 248}, {148, 123, 173}, {31, 158, 239}}, {{125, 81, 34}, {179, 100, 40}, {59, 156, 206}}, {{252, 199, 253}, {38, 117, 54}, {220, 120, 156}}});
    SeamCarver carver(test);
    std::vector<size_t> hor = carver.FindHorizontalSeam();
    std::vector<size_t> vert = carver.FindVerticalSeam();
    std::cout << "energyMap : " << std::endl;
    for (size_t i = 0; i < test.m_table.size(); i++) {
        for (size_t j = 0; j < test.m_table[0].size(); j++) {
            std::cout << carver.GetPixelEnergy(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "horSeam : ";
    for (const auto i : hor) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "vertSeam : ";
    for (const auto i : vert) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char * argv[])
{
    //regressiontest();

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