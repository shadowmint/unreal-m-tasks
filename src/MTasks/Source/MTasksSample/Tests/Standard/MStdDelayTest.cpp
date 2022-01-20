#include "Actors/MStdExecutor.h"
#include "MTasks/Public/Standard/MStdDelay.h"
#include "MTasksSample/Tests/Internal/MTestUtils.h"
#include "Standard/MStdResult.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MStdDelayTest, "Tests.Standard.MStdDelayTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MStdDelayTest::RunTest(const FString& Parameters)
{
	auto* WorldObject = UMTestUtils::GetAnyGameWorld();

	/** Normal workflow */
	auto const A = UMStdDelay::StdDelay(WorldObject, -1, 2);
	auto const B = UMStdDelay::StdDelay(WorldObject, -1, 2);
	auto const C = UMStdDelay::StdDelay(WorldObject, 2, -1);
	auto ResolvedA = false;
	auto ResolvedB = false;
	auto ResolvedC = false;
	
	A->Then(EMTaskState::Resolved, B);
	B->Then(EMTaskState::Resolved, C);

	A->Update.AddLambda([&](const UMTask *Task)
	{
		ResolvedA = Task->State == EMTaskState::Resolved;
	});
	B->Update.AddLambda([&](const UMTask *Task)
	{
		ResolvedB = Task->State == EMTaskState::Resolved;
	});
	C->Update.AddLambda([&](const UMTask *Task)
	{
		ResolvedC = Task->State == EMTaskState::Resolved;
	});
	
	auto const Exec = AMStdExecutor::GetStdExecutor(WorldObject);
	Exec->SetDebug(true);
	A->Start(Exec);

	check(!ResolvedA);
	Exec->Tick(1.0);
	check(!ResolvedA);
	Exec->Tick(1.0);
	check(ResolvedA);
	check(!ResolvedB);
	Exec->Tick(1.0);
	check(!ResolvedB);
	Exec->Tick(1.0);
	check(ResolvedB);
	check(!ResolvedC);
	Exec->Tick(1.0);
	check(!ResolvedC);
	Exec->Tick(1.0);
	check(ResolvedC);

	/** Timeout workflow */
	auto const D = UMStdDelay::StdDelay(WorldObject, 200, -1);
	auto const E = UMStdResult::Resolved(WorldObject);
	auto const F = UMStdResult::Resolved(WorldObject);
	auto RejectedD = false;
	auto ResolvedE = false;
	auto ResolvedF = false;
	
	D->Then(EMTaskState::Resolved, E);
	D->Then(EMTaskState::Rejected, F);

	D->Update.AddLambda([&](const UMTask *Task)
	{
		RejectedD = Task->State == EMTaskState::Rejected;
	});
	E->Update.AddLambda([&](const UMTask *Task)
	{
		ResolvedE = Task->State == EMTaskState::Resolved;
	});
	F->Update.AddLambda([&](const UMTask *Task)
	{
		ResolvedF = Task->State == EMTaskState::Resolved;
	});
	
	D->Start(Exec);

	// Remember, timeout is 60
	Exec->Tick(10.0);
	Exec->Tick(10.0);
	Exec->Tick(90.0);
	Exec->Tick(10.0);
	check(RejectedD);
	check(!ResolvedE); // Resolved child is not invoked
	check(ResolvedF); // Rejected child is invoked
	
	// Make the test pass by returning true, or fail by returning false.
	return true;
}
