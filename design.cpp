#include <iostream>
#include <vector>
#include <string>

struct ScanPoint { float distance; float angle; };
using Scan = std::vector<ScanPoint>;




/*
class LidarProcessor
{
public:
    Scan processScan(const std::string& filterType, bool denoise)
    {
        Scan scan;

        if (filterType == "median")
        {
            std::cout << "Median filter\n";
            scan = { {1.0f, 0.5f}, {2.0f, 1.0f} };
        }
        else if (filterType == "gaussian")
        {
            std::cout << "Gaussian filter\n";
            scan = { {1.1f, 0.5f}, {2.1f, 1.0f} };
        }
        else
        {
            std::cout << "No filter\n";
            scan = { {1.5f, 0.5f}, {2.5f, 1.0f} };
        }

        if (denoise)
        {
            std::cout << "Denoising\n";
        }

        return scan;
    }
};

class ObstacleDetector
{
public:
    void detect(const Scan& scan)
    {
        for (auto& p : scan)
            std::cout << "Obstacle at d=" << p.distance
            << " a=" << p.angle << "\n";
    }
};

*/

// rozhranie
class filter {
public :
    virtual ~filter() = default;
    virtual Scan filterType() = 0;


};

// kontext 

class LidarProcesor
{
private:
    std::unique_ptr<filter> filter_;


public:
    void SetFilter(std::unique_ptr<filter> p) {

        filter_ = std::move(p);
    }
    Scan processScan() {
        return filter_->filterType();
    }
};

class gaussian : public filter {

public:
    Scan filterType() override {


        std::cout << "gaussian filter\n";
        return { {1.0f, 0.5f}, {2.0f, 1.0f} };
    }

};

class median : public filter {

public:
    Scan filterType() override {


        std::cout << "Median filter\n";
        return { {1.1f, 0.5f}, {2.1f, 1.0f} };
    }

};

class denoise : public filter {
public:
    denoise(std::unique_ptr<filter> p)
        : inner_(std::move(p)) {

    }
    Scan filterType() override
    {
        Scan scan = inner_->filterType();  
        std::cout << "Denoising\n";        
        return scan;                        
    }
private:
    std::unique_ptr<filter> inner_;
};





int main()
{
    LidarProcesor proc;

    // gaussian BEZ denoisingu
    proc.SetFilter(std::make_unique<gaussian>());
    auto scan1 = proc.processScan();

    // median BEZ denoisingu
    proc.SetFilter(std::make_unique<median>());
    auto scan2 = proc.processScan();

    // gaussian S denoisingom (decorator)
    proc.SetFilter(
        std::make_unique<denoise>(std::make_unique<gaussian>())
    );
    auto scan3 = proc.processScan();

    return 0;
}
