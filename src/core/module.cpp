#include "medusa/module.hpp"
#include "medusa/log.hpp"

#include <algorithm>

#include <boost/filesystem.hpp>

MEDUSA_NAMESPACE_BEGIN

bool ModuleManager::LoadCommonFromModule(void * const pMod, Module & rModule)
{
  auto pGetArchitecture = rModule.Load<TGetArchitecture>(pMod, "GetArchitecture");
  if (pGetArchitecture != nullptr)
  {
    return ActionLoad<medusa::Architecture*>(pGetArchitecture(),
                                             "is an architecture", "unable to get architecture",
                                             [this](medusa::Architecture* const pArchitecture)
                                             {
                                                 Architecture::SPType ArchitecturePtr(pArchitecture);
                                                 m_Architectures.push_back(ArchitecturePtr);
                                             });
  }

  auto pGetBinding = rModule.Load<TGetBinding>(pMod, "GetBinding");
  if (pGetBinding != nullptr)
  {
    return ActionLoad<medusa::Binding*>(pGetBinding(), "is a binding", "unable to get binding",
                                        [this, pGetBinding](medusa::Binding* const pBinding)
                                        {
                                            m_Bindings[pBinding->GetName()] = pGetBinding;
                                            delete pBinding;
                                        });
  }

  auto pGetDatabase = rModule.Load<TGetDatabase>(pMod, "GetDatabase");
  if (pGetDatabase != nullptr)
  {
    return ActionLoad<medusa::Database*>(pGetDatabase(),
                                         "is a database", "unable to get database",
                                         [this](medusa::Database* const pDatabase)
                                         {
                                             Database::SPType spDatabase(pDatabase);
                                             m_Databases.push_back(spDatabase);
                                             return true;
                                         });
  }

  auto pGetEmulator = rModule.Load<TGetEmulator>(pMod, "GetEmulator");
  if (pGetEmulator != nullptr)
  {
    return ActionLoad<medusa::Emulator*>(pGetEmulator(nullptr, nullptr, nullptr),
                                         "is an emulator", "unable to get emulator",
                                         [this, pGetEmulator](medusa::Emulator* const pEmulator)
                                         {
                                             m_Emulators[pEmulator->GetName()] = pGetEmulator;
                                             delete pEmulator;
                                         });
  }

  auto pGetOperatingSystem = rModule.Load<TGetOperatingSystem>(pMod, "GetOperatingSystem");
  if (pGetOperatingSystem != nullptr)
  {
    return ActionLoad<medusa::OperatingSystem*>(pGetOperatingSystem(),
                                                "is an operating system", "unable to get operating system",
                                                [this](medusa::OperatingSystem* const pOperatingSystem)
                                                {
                                                    OperatingSystem::SPType spOperatingSystem(pOperatingSystem);
                                                    m_OperatingSystems.push_back(spOperatingSystem);
                                                });
  }
  return false;
}

void ModuleManager::LoadDatabases(boost::filesystem::path const& rModPath)
{
  try
  {
    const boost::filesystem::path CurDir = rModPath;
    Module Module;

    Log::Write("core") << "Module directory: " << boost::filesystem::system_complete(CurDir) << LogEnd;

    boost::filesystem::directory_iterator End;
    for (boost::filesystem::directory_iterator It(CurDir);
      It != End; ++It) {
      auto const &rFilename = It->path().string();

      if (rFilename.substr(rFilename.find_last_of(".") + 1) != Module::GetExtension())
        continue;

      auto FullPath = boost::filesystem::system_complete(*It);
      Log::Write("core") << "Module: \"" << rFilename << "\" ";

      void *pMod = Module.Load(FullPath);
      if (pMod == nullptr) {
        Log::Write("core") << "can't be loaded" << LogEnd;
        continue;
      }

      if (LoadCommonFromModule(pMod, Module) == false)
        Log::Write("core") << "is unknown (ignored)" << LogEnd;
    }
  }
  catch (std::exception &e)
  {
    Log::Write("core") << e.what() << LogEnd;
  }
}

void ModuleManager::LoadModules(boost::filesystem::path const& rModPath, BinaryStream const& rBinStrm)
{
  try
  {
    const boost::filesystem::path CurDir = rModPath;
    Module Module;

    Log::Write("core") << "Module directory: " << boost::filesystem::system_complete(CurDir) << LogEnd;

    boost::filesystem::directory_iterator End;
    for (boost::filesystem::directory_iterator It(CurDir);
      It != End; ++It)
    {
      auto const& rFilename = It->path().string();

      if (rFilename.substr(rFilename.find_last_of(".") + 1) != Module::GetExtension())
        continue;

      auto FullPath = boost::filesystem::system_complete(*It).string();
      Log::Write("core") << "Module: \"" << rFilename << "\" ";

      void* pMod = Module.Load(FullPath);
      if (pMod == nullptr)
      {
        Log::Write("core") << "can't be loaded" << LogEnd;
        continue;
      }

      TGetLoader pGetLoader = Module.Load<TGetLoader>(pMod, "GetLoader");
      if (pGetLoader != nullptr)
      {
        Log::Write("core") << "is a loader ";

        Loader* pLoader = pGetLoader();
        if (pLoader->IsCompatible(rBinStrm))
        {
          Loader::SPType LoaderPtr(pLoader);
          m_Loaders.push_back(LoaderPtr);
          Log::Write("core") << "(loaded)" << LogEnd;
        }
        else
        {
          Log::Write("core") << "(unloaded)" << LogEnd;
          delete pLoader;
        }
        continue;
      }

      if (LoadCommonFromModule(pMod, Module) == false)
        Log::Write("core") << "is unknown (ignored)" << LogEnd;
    }
  }
  catch (std::exception &e)
  {
    Log::Write("core") << e.what() << LogEnd;
  }

  if (!m_Loaders.empty())
    std::sort(std::begin(m_Loaders), std::end(m_Loaders), [](Loader::SPType spLdr0, Loader::SPType spLdr1)
        {
          return spLdr0->GetDepth() > spLdr1->GetDepth();
        });
}

void ModuleManager::UnloadModules(void)
{
  m_DefaultArchitectureTag = MEDUSA_ARCH_UNK;
  m_Loaders.clear();
  m_Architectures.clear();
  m_Databases.clear();
  m_OperatingSystems.clear();
  m_Emulators.clear();
}

Architecture::SPType ModuleManager::GetArchitecture(Tag ArchTag) const
{
  if (ArchTag == MEDUSA_ARCH_UNK) {
    ArchTag = m_DefaultArchitectureTag;
  }

  // UPDATE replace :
  //auto itArch = m_TaggedArchitectures.find(ArchTag);
  auto itArch = std::find_if(std::begin(m_TaggedArchitectures),
                             std::end(m_TaggedArchitectures),
                             [ArchTag](const std::pair<const unsigned int, std::shared_ptr<medusa::Architecture> > arch)
                             { return (arch.first & ArchTag) == ArchTag;});

  if (itArch == std::end(m_TaggedArchitectures))
    return Architecture::SPType();
  return itArch->second;
}

Architecture::SPType ModuleManager::FindArchitecture(Tag ArchTag) const
{
  if (ArchTag == MEDUSA_ARCH_UNK)
    return nullptr;

  auto const& rAllArchs = GetArchitectures();
  for (auto itArch = std::begin(rAllArchs), itEnd = std::end(rAllArchs); itArch != itEnd; ++itArch)
    if (MEDUSA_CMP_TAG((*itArch)->GetTag(), ArchTag))
      return *itArch;
  return nullptr;
}

bool ModuleManager::RegisterArchitecture(Architecture::SPType spArch)
{
  u8 Id = 0;
  bool FoundId = false;
  for (u8 i = 0; i < 32; ++i)
    if (!(m_ArchIdPool & (1 << i)))
    {
      m_ArchIdPool |= (1 << i);
      Id = i;
      FoundId = true;
      break;
    }

  if (FoundId == false)
    return false;

  spArch->UpdateId(Id);

  m_TaggedArchitectures[spArch->GetTag()] = spArch;

  if (m_DefaultArchitectureTag == MEDUSA_ARCH_UNK)
    m_DefaultArchitectureTag = spArch->GetTag();
  return true;
}

// TODO: test this function
bool ModuleManager::UnregisterArchitecture(Architecture::SPType spArch)
{
  Tag ArchTag = spArch->GetTag();
  auto itArch = std::find_if(std::begin(m_TaggedArchitectures),
                             std::end(m_TaggedArchitectures),
                             [ArchTag](const std::pair<const unsigned int,
                                       std::shared_ptr<medusa::Architecture> > arch)
                             {
                                 return (arch.first & ArchTag) == ArchTag;
                             });

  if (itArch == std::end(m_TaggedArchitectures))
    return false;
  m_TaggedArchitectures.erase(itArch);
  return true; /* Not implemented */
}

void ModuleManager::ResetArchitecture(void)
{
  m_Architectures.clear();
}

TGetBinding ModuleManager::GetBinding(std::string const& rBindingName)
{
  auto itBinding = m_Bindings.find(rBindingName);
  if (itBinding == std::end(m_Bindings))
    return nullptr;
  return itBinding->second;
}

TGetEmulator ModuleManager::GetEmulator(std::string const& rEmulatorName)
{
  auto itEmulator = m_Emulators.find(rEmulatorName);
  if (itEmulator == std::end(m_Emulators))
    return nullptr;
  return itEmulator->second;
}

OperatingSystem::SPType ModuleManager::GetOperatingSystem(std::string const& rOperatingSystemName) const
{
  for (auto spOs : m_OperatingSystems)
    if (spOs->GetName() == rOperatingSystemName)
      return spOs;
  return nullptr;
}

OperatingSystem::SPType ModuleManager::GetOperatingSystem(Loader::SPType spLdr, Architecture::SPType spArch) const
{
  OperatingSystem::VSPType CompatOs;
  for (auto itOs = std::begin(m_OperatingSystems); itOs != std::end(m_OperatingSystems); ++itOs)
    if ((*itOs)->IsSupported(*spLdr, *spArch) == true)
      return *itOs;
  return OperatingSystem::SPType();
}

Database::SPType ModuleManager::GetDatabase(std::string const& rDatabaseName)
{
  for (auto itDb: m_Databases)
    if (itDb->GetName() == rDatabaseName)
      return itDb;
  return Database::SPType();
}

Database::VSPType ModuleManager::GetDatabases(void) const
{
  return m_Databases;
}

Loader::VSPType ModuleManager::GetLoaders(void) const
{
  return m_Loaders;
}

Architecture::VSPType ModuleManager::GetArchitectures(void) const
{
  return m_Architectures;
}

MEDUSA_NAMESPACE_END
