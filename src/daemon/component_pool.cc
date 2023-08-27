#include "component_pool.h"

#include "database/public.h"
#include "core/common.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    ComponentPool* pComponentPool;

    void InitComponentPool()
    {
        pComponentPool = new ComponentPool;
    }

    ComponentPool::ComponentPool()
    {
        components_alias_[Component::kDataBase] = "DataBase";
        components_alias_[Component::kCore] = "Core";
        components_alias_[Component::kLog] = "Log";
        components_alias_[Component::kMail] = "Mail";
        components_alias_[Component::kMonitor] = "Monitor";
    }

    ComponentPool::~ComponentPool()
    {
        for (auto &pcompo : components_)
            delete pcompo.second;
    }

    utility::os::LocalProcess &ComponentPool::operator[](Component component)
    {
        return *(components_[component]);
    }

    utility::os::LocalProcess* ComponentPool::Find(pid_t pid)
    {
        utility::os::LocalProcess* pProc {nullptr};

        for (auto &comp : components_)
        {
            if(comp.second->GetID() == pid)
            {
                pProc = comp.second;
                break;
            }
        }
        return pProc;
    }


    void ComponentPool::Stop()
    {
        using utility::os::LocalProcess;

        auto stop_one = [this](Component compo, int signo)
        {
            auto pProc = this->components_[compo];
            if (pProc->GetStatus() == LocalProcess::Status::kRunning)
                pProc->SendSignal(signo);
        };

        stop_one(Component::kDataBase, database::kTerminateSignal);
        stop_one(Component::kCore, core::kTermianteSignal); 

        //TODO: Finish all components and stop_one for each.
    }
} // namespace daemon

_END_EXPRESS_NAMESPACE_