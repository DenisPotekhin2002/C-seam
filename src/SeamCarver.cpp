#include "SeamCarver.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>

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
    return (m_image.m_table.empty() ? 0 : m_image.m_table[0].size());
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
    return std::sqrt(xr * xr + xg * xg + xb * xb + yr * yr + yg * yg + yb * yb);
}

SeamCarver::Seam SeamCarver::find(size_t width, size_t height, const std::function<double(size_t, size_t)> & energy) const
{
    std::vector<std::vector<double>> energies;
    std::vector<std::vector<int>> ancestors;
    for (size_t col = 0; col < width; col++) {
        energies.push_back(std::vector<double>());
        ancestors.push_back(std::vector<int>());
        for (size_t row = 0; row < height; row++) {
            energies[col].push_back(0.0);
            ancestors[col].push_back(0);
        }
    }
    for (size_t row = 0; row < height; row++) {
        energies[0][row] = energy(0, row);
        ancestors[0][row] = 0;
    }
    for (size_t col = 1; col < width; col++) {
        for (size_t row = 0; row < height; row++) {
            double temp = energies[col - 1][row];
            double tempanc = row;
            if (row > 0 && energies[col - 1][row - 1] < temp) {
                temp = energies[col - 1][row - 1];
                tempanc = row - 1;
            }
            if (row < height - 1 && energies[col - 1][row + 1] < temp) {
                temp = energies[col - 1][row + 1];
                tempanc = row + 1;
            }
            energies[col][row] = temp + energy(col, row);
            ancestors[col][row] = tempanc;
        }
    }
    int res = std::min_element(energies[width - 1].begin(), energies[width - 1].end()) - energies[width - 1].begin();
    Seam resseam;
    while (resseam.size() < width) {
        resseam.push_back(res);
        res = ancestors[width - resseam.size()][res];
    }
    std::reverse(resseam.begin(), resseam.end());
    return resseam;
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    return find(GetImageWidth(), GetImageHeight(), [this](size_t x, size_t y) { return GetPixelEnergy(x, y); });
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    return find(GetImageHeight(), GetImageWidth(), [this](size_t x, size_t y) { return GetPixelEnergy(y, x); });
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
