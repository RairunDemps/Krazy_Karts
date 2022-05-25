#pragma once

class KKUtils
{
public:
    static FString GetEnumRoleString(ENetRole Role)
    {
        switch (Role)
        {
            case ROLE_None:
                return TEXT("None");
            case ROLE_SimulatedProxy:
                return TEXT("SimulatedProxy");
            case ROLE_AutonomousProxy:
                return TEXT("AutonomousProxy");
            case ROLE_Authority:
                return TEXT("Authority");
            default:
                return TEXT("ERROR");
        }
    }
};
