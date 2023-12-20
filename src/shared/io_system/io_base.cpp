
/**
 * @file 	io_base.cpp
 * @author	Luhui Han, Chi Zhang and Xiangyu Hu
 */

#include "io_base.h"

#include "sph_system.h"

namespace SPH
{
//=============================================================================================//
BaseIO::BaseIO(SPHSystem &sph_system)
    : sph_system_(sph_system), io_environment_(*sph_system.io_environment_) {}
//=============================================================================================//
std::string BaseIO::convertPhysicalTimeToString(Real convertPhysicalTimeToStream)
{
    int i_time = int(GlobalStaticVariables::physical_time_ * 1.0e6);
    return padValueWithZeros(i_time);
}
//=============================================================================================//
BodyStatesRecording::BodyStatesRecording(SPHBodyVector bodies)
    : BaseIO(bodies[0]->getSPHSystem()), bodies_(bodies),
      state_recording_(sph_system_.StateRecording()) {}
//=============================================================================================//
BodyStatesRecording::BodyStatesRecording(SPHBody &body)
    : BodyStatesRecording({&body}) {}
//=============================================================================================//
void BodyStatesRecording::writeToFile()
{
    writeWithFileName(convertPhysicalTimeToString(GlobalStaticVariables::physical_time_));
}
//=============================================================================================//
void BodyStatesRecording::writeToFile(size_t iteration_step)
{
    writeWithFileName(padValueWithZeros(iteration_step));
};
//=============================================================================================//
RestartIO::RestartIO(SPHBodyVector bodies)
    : BaseIO(bodies[0]->getSPHSystem()), bodies_(bodies),
      overall_file_path_(io_environment_.restart_folder_ + "/Restart_time_")
{
    std::transform(bodies.begin(), bodies.end(), std::back_inserter(file_names_),
                   [&](SPHBody *body) -> std::string
                   { return io_environment_.restart_folder_ + "/" + body->getName() + "_rst_"; });
}
//=============================================================================================//
void RestartIO::writeToFile(size_t iteration_step)
{
    std::string overall_filefullpath = overall_file_path_ + padValueWithZeros(iteration_step) + ".dat";
    if (fs::exists(overall_filefullpath))
    {
        fs::remove(overall_filefullpath);
    }
    std::ofstream out_file(overall_filefullpath.c_str(), std::ios::app);
    out_file << std::fixed << std::setprecision(9) << GlobalStaticVariables::physical_time_ << "   \n";
    out_file.close();

    for (size_t i = 0; i < bodies_.size(); ++i)
    {
        std::string filefullpath = file_names_[i] + padValueWithZeros(iteration_step) + ".xml";

        if (fs::exists(filefullpath))
        {
            fs::remove(filefullpath);
        }
        bodies_[i]->writeParticlesToXmlForRestart(filefullpath);
    }
}
//=============================================================================================//
Real RestartIO::readRestartTime(size_t restart_step)
{
    std::cout << "\n Reading restart files from the restart step = " << restart_step << std::endl;
    std::string overall_filefullpath = overall_file_path_ + padValueWithZeros(restart_step) + ".dat";
    if (!fs::exists(overall_filefullpath))
    {
        std::cout << "\n Error: the input file:" << overall_filefullpath << " is not exists" << std::endl;
        std::cout << __FILE__ << ':' << __LINE__ << std::endl;
        exit(1);
    }
    Real restart_time;
    std::ifstream in_file(overall_filefullpath.c_str());
    in_file >> restart_time;
    in_file.close();

    return restart_time;
}
//=============================================================================================//
void RestartIO::readFromFile(size_t restart_step)
{
    for (size_t i = 0; i < bodies_.size(); ++i)
    {
        std::string filefullpath = file_names_[i] + padValueWithZeros(restart_step) + ".xml";

        if (!fs::exists(filefullpath))
        {
            std::cout << "\n Error: the input file:" << filefullpath << " is not exists" << std::endl;
            std::cout << __FILE__ << ':' << __LINE__ << std::endl;
            exit(1);
        }

        bodies_[i]->readParticlesFromXmlForRestart(filefullpath);
    }
}
//=============================================================================================//
ReloadParticleIO::ReloadParticleIO(SPHBodyVector bodies)
    : BaseIO(bodies[0]->getSPHSystem()), bodies_(bodies)
{
    std::transform(bodies.begin(), bodies.end(), std::back_inserter(file_names_),
                   [&](SPHBody *body) -> std::string
                   { return io_environment_.reload_folder_ + "/" + body->getName() + "_rld.xml"; });
}
//=============================================================================================//
ReloadParticleIO::ReloadParticleIO(SPHBody &sph_body, const std::string &given_body_name)
    : BaseIO(sph_body.getSPHSystem()), bodies_({&sph_body})
{
    file_names_.push_back(io_environment_.reload_folder_ + "/" + given_body_name + "_rld.xml");
}
//=============================================================================================//
ReloadParticleIO::ReloadParticleIO(SPHBody &sph_body)
    : ReloadParticleIO(sph_body, sph_body.getName()) {}
//=============================================================================================//
void ReloadParticleIO::writeToFile(size_t iteration_step)
{
    for (size_t i = 0; i < bodies_.size(); ++i)
    {
        std::string filefullpath = file_names_[i];

        if (fs::exists(filefullpath))
        {
            fs::remove(filefullpath);
        }
        bodies_[i]->writeToXmlForReloadParticle(filefullpath);
    }
}
//=============================================================================================//
void ReloadParticleIO::readFromFile(size_t restart_step)
{
    std::cout << "\n Reloading particles from files." << std::endl;
    for (size_t i = 0; i < bodies_.size(); ++i)
    {
        std::string filefullpath = file_names_[i];

        if (!fs::exists(filefullpath))
        {
            std::cout << "\n Error: the input file:" << filefullpath << " is not exists" << std::endl;
            std::cout << __FILE__ << ':' << __LINE__ << std::endl;
            exit(1);
        }

        bodies_[i]->readFromXmlForReloadParticle(filefullpath);
    }
}
//=================================================================================================//
} // namespace SPH
