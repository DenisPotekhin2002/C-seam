#include "SeamCarver.h"

#include <cmath>

SeamCarver::SeamCarver(Image image)
    : m_image(std::move(image))
{
}

const Image & SeamCarver::GetImage() const
{
    return m_image;
}

size_t SeamCarver::GetImageWidth() const
{
    return m_image.m_table.size();
}

size_t SeamCarver::GetImageHeight() const
{
    return m_image.m_table[0].size();
}

double SeamCarver::GetPixelEnergy(size_t columnId, size_t rowId) const
{
    if (columnId >= GetImageWidth() || rowId >= GetImageHeight()) {
        return 0;
    }
    Image::Pixel x1 = m_image.GetPixel(columnId, rowId);
    Image::Pixel x2 = m_image.GetPixel(columnId, rowId);
    Image::Pixel y1 = m_image.GetPixel(columnId, rowId);
    Image::Pixel y2 = m_image.GetPixel(columnId, rowId);
    if (columnId < GetImageWidth() - 1) {
        x1 = m_image.GetPixel(columnId + 1, rowId);
    }
    else {
        x1 = m_image.GetPixel(0, rowId);
    }
    if (columnId > 0) {
        x2 = m_image.GetPixel(columnId - 1, rowId);
    }
    else {
        x2 = m_image.GetPixel(GetImageWidth() - 1, rowId);
    }
    if (rowId < GetImageHeight() - 1) {
        y1 = m_image.GetPixel(columnId, rowId + 1);
    }
    else {
        y1 = m_image.GetPixel(columnId, 0);
    }
    if (rowId > 0) {
        y2 = m_image.GetPixel(columnId, rowId - 1);
    }
    else {
        y2 = m_image.GetPixel(columnId, GetImageHeight() - 1);
    }
    size_t xr = x1.m_red - x2.m_red;
    size_t xg = x1.m_green - x2.m_green;
    size_t xb = x1.m_blue - x2.m_blue;
    size_t yr = y1.m_red - y2.m_red;
    size_t yg = y1.m_green - y2.m_green;
    size_t yb = y1.m_blue - y2.m_blue;
    return sqrt(xr * xr + xg * xg + xb * xb + yr * yr + yg * yg + yb * yb);
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    std::vector<std::vector<double>> energies;
    std::vector<std::vector<std::vector<size_t>>> seams;
    for (size_t col = 0; col < GetImageWidth(); col++) {
        energies.push_back(std::vector<double>());
        seams.push_back(std::vector<std::vector<size_t>>());
        for (size_t row = 0; row < GetImageHeight(); row++) {
            energies[col].push_back(0);
            seams[col].push_back(std::vector<size_t>());
        }
    }
    for (size_t row = 0; row < GetImageHeight(); row++) {
        energies[0][row] = GetPixelEnergy(0, row);
        seams[0][row].push_back(row);
    }
    for (size_t col = 1; col < GetImageWidth(); col++) {
        for (size_t row = 0; row < GetImageHeight(); row++) {
            double temp = energies[col - 1][row];
            std::vector tempseam = seams[col - 1][row];
            if (row > 0 && energies[col - 1][row - 1] < temp) {
                temp = energies[col - 1][row - 1];
                tempseam = seams[col - 1][row - 1];
            }
            if (row < GetImageHeight() - 1 && energies[col - 1][row + 1] < temp) {
                temp = energies[col - 1][row + 1];
                tempseam = seams[col - 1][row + 1];
            }
            energies[col][row] = temp + GetPixelEnergy(col, row);
            seams[col][row] = tempseam;
            seams[col][row].push_back(row);
        }
    }
    int res = std::numeric_limits<int>::max();
    Seam resseam = Seam();
    for (size_t row = 0; row < GetImageHeight(); row++) {
        if (energies[GetImageWidth() - 1][row] < res) {
            res = energies[GetImageWidth() - 1][row];
            resseam = seams[GetImageWidth() - 1][row];
        }
    }
    return resseam;
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    std::vector<std::vector<double>> energies;
    std::vector<std::vector<std::vector<size_t>>> seams;
    for (size_t row = 0; row < GetImageHeight(); row++) {
        energies.push_back(std::vector<double>());
        seams.push_back(std::vector<std::vector<size_t>>());
        for (size_t col = 0; col < GetImageWidth(); col++) {
            energies[row].push_back(0);
            seams[row].push_back(std::vector<size_t>());
        }
    }
    for (size_t col = 0; col < GetImageWidth(); col++) {
        energies[0][col] = GetPixelEnergy(col, 0);
        seams[0][col].push_back(col);
    }
    for (size_t row = 1; row < GetImageHeight(); row++) {
        for (size_t col = 0; col < GetImageWidth(); col++) {
            double temp = energies[row - 1][col];
            std::vector tempseam = seams[row - 1][col];
            if (col > 0 && energies[row - 1][col - 1] < temp) {
                temp = energies[row - 1][col - 1];
                tempseam = seams[row - 1][col - 1];
            }
            if (col < GetImageWidth() - 1 && energies[row - 1][col + 1] < temp) {
                temp = energies[row - 1][col + 1];
                tempseam = seams[row - 1][col + 1];
            }
            energies[row][col] = temp + GetPixelEnergy(col, row);
            seams[row][col] = tempseam;
            seams[row][col].push_back(col);
        }
    }
    int res = std::numeric_limits<int>::max();
    Seam resseam = Seam();
    for (size_t col = 0; col < GetImageWidth(); col++) {
        if (energies[GetImageHeight() - 1][col] < res) {
            res = energies[GetImageHeight() - 1][col];
            resseam = seams[GetImageHeight() - 1][col];
        }
    }
    return resseam;
}

void SeamCarver::RemoveHorizontalSeam(const Seam & seam)
{
    if (!seam.empty()) {
        for (size_t i = 0; i < GetImageWidth(); i++) {
            for (size_t j = seam[i]; j < GetImageHeight() - 1; j++) {
                m_image.m_table[i][j] = m_image.m_table[i][j + 1];
            }
        }
        for (size_t w = 0; w < GetImageWidth(); w++) {
            m_image.m_table[w].pop_back();
        }
    }
}

void SeamCarver::RemoveVerticalSeam(const Seam & seam)
{
    if (!seam.empty()) {
        for (size_t i = 0; i < GetImageHeight(); i++) {
            for (size_t j = seam[i]; j < GetImageWidth() - 1; j++) {
                m_image.m_table[j][i] = m_image.m_table[j + 1][i];
            }
        }
        m_image.m_table.pop_back();
    }
}
