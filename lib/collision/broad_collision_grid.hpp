#pragma once

#include "mathematics/vector.hpp"
#include "objects/object.hpp"

#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>

struct CellKey
{
    int  x, y, z;
    bool operator==(const CellKey&) const = default;
};

/**
 * @brief Hash function for @ref CellKey, so cells can be stored in a `std::unordered_map`.
 *
 * Each integer coordinate is multiplied by a large prime and the results are combined with XOR,
 * which spreads neighbouring cells across the table. Two different cells may share a hash
 * (collision): the map then tells them apart with `CellKey::operator==`, so results stay correct,
 * only lookups get slower.
 */
struct CellKeyHash
{
    /**
     * @brief Compute the hash of a cell.
     * @param k Integer coordinates of the cell.
     * @return Hash value used by the map to pick a bucket.
     */
    size_t operator()(const CellKey& k) const
    {
        // classic spatial-hash primes (Teschner et al., 2003)
        return (static_cast<size_t>(k.x) * 73856093u) ^ (static_cast<size_t>(k.y) * 19349663u) ^
               (static_cast<size_t>(k.z) * 83492791u);
    }
};

struct BroadCollisionGrid
{
    // Test if objects are overlapping by creating a 3D grid of size equal to the largest object. Only
    // neighbours are tested : O(N), but issue if objects have a wide range of size.
private:
    decimal                                                       cellSize;
    std::unordered_map<CellKey, std::vector<size_t>, CellKeyHash> cells;
    std::vector<std::pair<size_t, size_t>>                        pairs;

public:
    decimal getCellSize(const std::vector<Object*>& objects) const;

    CellKey computeCell(const Vector3D& p) const;

    void computeGrid(const std::vector<Object*>& objects);

    void computePairs(const std::vector<Object*>& objects);

    /// Rebuild the grid from the current positions (call once per step)
    void update(const std::vector<Object*>& objects);

    /// Candidate pairs (i < j) found by the last update
    const std::vector<std::pair<size_t, size_t>>& getPairs() const { return pairs; }
};