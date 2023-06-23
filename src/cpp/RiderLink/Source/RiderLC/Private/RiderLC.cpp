﻿#include "RiderLC.hpp"

#include "IRiderLink.hpp"
#include "RdEditorModel/RdEditorModel.Generated.h"
#include "task/RdEndpoint.h"

#include "Async/Async.h"
#include "ILiveCodingModule.h"

#define LOCTEXT_NAMESPACE "FRiderLCModule"

DEFINE_LOG_CATEGORY(FLogRiderLCModule);


void WrapRDCall(rd::RdEndpoint<rd::Void, bool, rd::Polymorphic<rd::Void>, rd::Polymorphic<bool>> const & Call, TFunction<bool(const ILiveCodingModule&)> LocalCall)
{
	Call.set([LocalCall](rd::Void const&)-> bool
	{
		const ILiveCodingModule* LiveCodingModule = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME);
		if(LiveCodingModule != nullptr)
		{
			return LocalCall(*LiveCodingModule);
		}
		return false;
	});
}

void FRiderLCModule::SetupLiveCodingBinds()
{
	IRiderLinkModule& RiderLinkModule = IRiderLinkModule::Get();
	RiderLinkModule.ViewModel(ModuleLifetimeDef.lifetime, [](rd::Lifetime Lifetime, JetBrains::EditorPlugin::RdEditorModel const& RdEditorModel)
	{
		RdEditorModel.get_lC_Compile().advise(Lifetime, []
		{
			AsyncTask(ENamedThreads::GameThread, []
			{
				ILiveCodingModule* LiveCodingModule = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME);
				if(LiveCodingModule != nullptr)
				{
					LiveCodingModule->Compile();
				}				
			});
		});

		RdEditorModel.get_lC_EnableByDefault().advise(Lifetime, [](bool Enable)
		{
			ILiveCodingModule* LiveCodingModule = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME);
			if(LiveCodingModule != nullptr)
			{
				LiveCodingModule->EnableByDefault(Enable);
			}			
		});

		RdEditorModel.get_lC_EnableForSession().advise(Lifetime, [](bool Enable)
		{
			ILiveCodingModule* LiveCodingModule = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME);
			if(LiveCodingModule != nullptr)
			{
				LiveCodingModule->EnableForSession(Enable);
			}			
		});
		
		WrapRDCall(RdEditorModel.get_lC_IsEnabledByDefault(), [](const ILiveCodingModule& LiveCodingModule)
		{
			return LiveCodingModule.IsEnabledByDefault();
		});
		
		WrapRDCall(RdEditorModel.get_lC_IsEnabledForSession(), [](const ILiveCodingModule& LiveCodingModule)
		{
			return LiveCodingModule.IsEnabledForSession();
		});
		
		WrapRDCall(RdEditorModel.get_lC_CanEnableForSession(), [](const ILiveCodingModule& LiveCodingModule)
		{
			return LiveCodingModule.CanEnableForSession();
		});

		RdEditorModel.get_lC_IsModuleStarted().fire(true);
	});
}

bool FRiderLCModule::Tick(float Delta)
{
	IRiderLinkModule& RiderLinkModule = IRiderLinkModule::Get();
	RiderLinkModule.QueueModelAction([](JetBrains::EditorPlugin::RdEditorModel const& RdEditorModel)
	{
		const ILiveCodingModule& LiveCodingModule = FModuleManager::GetModuleChecked<ILiveCodingModule>(LIVE_CODING_MODULE_NAME);
		RdEditorModel.get_lC_IsAvailable().set(LiveCodingModule.HasStarted() && LiveCodingModule.IsEnabledForSession());
		RdEditorModel.get_lC_IsCompiling().set(LiveCodingModule.IsCompiling());
	});

	return true;
}

void FRiderLCModule::StartupModule()
{
	UE_LOG(FLogRiderLCModule, Verbose, TEXT("RiderLC STARTUP START"));
	
	const IRiderLinkModule& RiderLinkModule = IRiderLinkModule::Get();
	ModuleLifetimeDef = RiderLinkModule.CreateNestedLifetimeDefinition();
	SetupLiveCodingBinds();
	TickDelegate = FTickerDelegate::CreateRaw(this, &FRiderLCModule::Tick);
#if ENGINE_MAJOR_VERSION < 5
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
#else
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);
#endif
	
	UE_LOG(FLogRiderLCModule, Verbose, TEXT("RiderLC STARTUP FINISH"));
}

void FRiderLCModule::ShutdownModule()
{
	UE_LOG(FLogRiderLCModule, Verbose, TEXT("RiderLC SHUTDOWN START"));

#if ENGINE_MAJOR_VERSION < 5
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
#else
	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
#endif
	ModuleLifetimeDef.terminate();
	
	UE_LOG(FLogRiderLCModule, Verbose, TEXT("RiderLC SHUTDOWN FINISH"));
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FRiderLCModule, RiderLC)