#include "precision.hpp"

#include <stdexcept>
#include <string>

struct Material
{
private:
    std::string name;
    decimal     density;
    decimal     stiffness;
    decimal     damping;
    decimal     friction;
    decimal     restitution = 0.5_d; // For simplified collision

public:
    /// @name Constructors
    /// @{

    Material()                           = default;
    Material(const Material&)            = default;
    Material& operator=(const Material&) = default;
    Material(std::string name, decimal density, decimal stiffness, decimal damping, decimal friction,
             decimal restitution)
        : name(std::move(name))
        , density(density)
        , stiffness(stiffness)
        , damping(damping)
        , friction(friction)
        , restitution(restitution)
    {}

    /// @}

    /// @name Getters
    /// @{

    Material&   get() const;
    std::string getName() const;
    decimal     getDensity() const;
    decimal     getStiffness() const;
    decimal     getDamping() const;
    decimal     getFriction() const;
    decimal     getRestitution() const;

    /// @}

    /// @name Setters
    /// @{

    void setName(std::string _name) { name = _name; }
    void setDensity(decimal _rho) { density = _rho; }
    void setStiffness(decimal _stiffness)
    {
        if (_stiffness < 0)
            throw std::invalid_argument("stiffness module cannot be negative");
        stiffness = _stiffness;
    }
    void setDamping(decimal _damping)
    {
        if (_damping < 0)
            throw std::invalid_argument("Damping cannot be negative");
        damping = _damping;
    }
    void setFriction(decimal _friction)
    {
        if (_friction < 0)
            throw std::invalid_argument("Friction cannot be negative");
        friction = _friction;
    }
    void setRestitution(decimal _restitution)
    {
        if (_restitution < 0)
            throw std::invalid_argument("Restitution factor cannot be negative");
        restitution = _restitution;
    }

    /// @}

    /// @name Loading Methods
    /// @{

    void loadFromFile(const std::string& path);

    /// @}
};