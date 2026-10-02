#pragma once

#include "CoreMinimal.h"
#include "PuzzleInteractable.h"
#include "PasswordKeypad.generated.h"

class ADARKCharacter;
class UUserWidget;

UCLASS()
class DARK_API APasswordKeypad : public APuzzleInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void Interact(ADARKCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable)
    void PasswordCompleted(bool bCorrect);

    UFUNCTION(BlueprintCallable)
    void ClosePasswordWidget();

protected:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Password")
    FString CorrectPassword = TEXT("90430");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Password")
    TSubclassOf<UUserWidget> PasswordWidgetClass;

private:

    UPROPERTY()
    UUserWidget* PasswordWidget = nullptr;

    UPROPERTY()
    ADARKCharacter* InteractingPlayer = nullptr;

    void OpenPasswordWidget();
};