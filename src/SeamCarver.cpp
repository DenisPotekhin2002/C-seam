#include "SeamCarver.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

#define find(width, height, energy)                                                                                      \
    std::vector<std::vector<double>> energies;                                                                           \
    std::vector<std::vector<std::vector<size_t>>> seams;                                                                 \
    for (size_t col = 0; col < width; col++) {                                                                           \
        energies.push_back(std::vector<double>());                                                                       \
        seams.push_back(std::vector<std::vector<size_t>>());                                                             \
        for (size_t row = 0; row < height; row++) {                                                                      \
            energies[col].push_back(0.0);                                                                                \
            seams[col].push_back(std::vector<size_t>());                                                                 \
        }                                                                                                                \
    }                                                                                                                    \
    for (size_t row = 0; row < height; row++) {                                                                          \
        energies[0][row] = energy(0, row);                                                                               \
        seams[0][row].push_back(row);                                                                                    \
    }                                                                                                                    \
    for (size_t col = 1; col < width; col++) {                                                                           \
        for (size_t row = 0; row < height; row++) {                                                                      \
            double temp = energies[col - 1][row];                                                                        \
            std::vector tempseam = seams[col - 1][row];                                                                  \
            if (row > 0 && energies[col - 1][row - 1] < temp) {                                                          \
                temp = energies[col - 1][row - 1];                                                                       \
                tempseam = seams[col - 1][row - 1];                                                                      \
            }                                                                                                            \
            if (row < height - 1 && energies[col - 1][row + 1] < temp) {                                                 \
                temp = energies[col - 1][row + 1];                                                                       \
                tempseam = seams[col - 1][row + 1];                                                                      \
            }                                                                                                            \
            energies[col][row] = temp + energy(col, row);                                                                \
            seams[col][row] = tempseam;                                                                                  \
            seams[col][row].push_back(row);                                                                              \
        }                                                                                                                \
    }                                                                                                                    \
    double res = std::min_element(energies[width - 1].begin(), energies[width - 1].end()) - energies[width - 1].begin(); \
    Seam resseam = seams[width - 1][res];                                                                                \
    return resseam;

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
    if (m_image.m_table.empty()) {
        return 0;
    }
    else {
        return m_image.m_table[0].size();
    }
}

double SeamCarver::GetPixelEnergy(size_t columnId, size_t rowId) const
{
    if (columnId >= GetImageWidth() || rowId >= GetImageHeight()) {
        return 0;
    }
    const Image::Pixel x1 = columnId < GetImageWidth() - 1 ? m_image.GetPixel(columnId + 1, rowId) : m_image.GetPixel(0, rowId);
    const Image::Pixel x2 = columnId > 0 ? m_image.GetPixel(columnId - 1, rowId) : m_image.GetPixel(GetImageWidth() - 1, rowId);
    const Image::Pixel y1 = rowId < GetImageHeight() - 1 ? m_image.GetPixel(columnId, rowId + 1) : m_image.GetPixel(columnId, 0);
    const Image::Pixel y2 = rowId > 0 ? m_image.GetPixel(columnId, rowId - 1) : m_image.GetPixel(columnId, GetImageHeight() - 1);
    double xr = x1.m_red - x2.m_red;
    double xg = x1.m_green - x2.m_green;
    double xb = x1.m_blue - x2.m_blue;
    double yr = y1.m_red - y2.m_red;
    double yg = y1.m_green - y2.m_green;
    double yb = y1.m_blue - y2.m_blue;
    return sqrt(xr * xr + xg * xg + xb * xb + yr * yr + yg * yg + yb * yb);
}

double SeamCarver::GetInversePixelEnergy(size_t rowId, size_t columnId) const
{
    return GetPixelEnergy(columnId, rowId);
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    find(GetImageWidth(), GetImageHeight(), GetPixelEnergy);
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    find(GetImageHeight(), GetImageWidth(), GetInversePixelEnergy);
}

void SeamCarver::RemoveHorizontalSeam(const Seam & seam)
{
    assert(seam.size() == GetImageWidth());
    for (size_t i = 0; i < GetImageWidth(); i++) {
        m_image.m_table[i].erase(m_image.m_table[i].begin() + seam[i]);
    }
}

void SeamCarver::RemoveVerticalSeam(const Seam & seam)
{
    assert(seam.size() == GetImageHeight());
    for (size_t i = 0; i < GetImageHeight(); i++) {
        for (size_t j = seam[i]; j < GetImageWidth() - 1; j++) {
            m_image.m_table[j][i] = m_image.m_table[j + 1][i];
        }
    }
    m_image.m_table.pop_back();
}
