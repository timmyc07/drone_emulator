#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProbeActor.generated.h"

UCLASS()
class AP0ProbeActor : public AActor
{
    GENERATED_BODY()
public:
    AP0ProbeActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    int32 ProbeTicks = 0;
};
