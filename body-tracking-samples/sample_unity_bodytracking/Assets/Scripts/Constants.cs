class Constants
{
    public const float Invalid2DCoordinate = -1;

    public const int InvalidCalculationWindow = -1;

    public const ulong InvalidBodySelectionIndex = ulong.MaxValue;

    public class Validation
    {
        public class ErrorMessages
        {
            public const string InitialStateStability = "Please stand upright in the beginning of the jump";
            public const string EndingStateStability = "Please stand upright in the end of the jump";

            public const string MovementDisplacementHorizontal = "Oops, you need to land where you started";
            public const string MovementDisplacementVertical = "Please stand upright during the jump";

            public const string HandsDisplacement = "Please keep hands on hips while you jump";

            public const string AngleDisplacement = "Please stand upright in the beginning of the jump";
     
            public const string MaximalHeight = "Please jump again";

            public const string NotEnoughPoints = "Jump does not have sufficient number of points";
        }
    }

    public class TextColor
    {
        public CustomColors green = new CustomColors(0.2f, 0.2f, 0.2f);
        public CustomColors yellow = new CustomColors(0.2f, 0.2f, 0.2f);
    }
}


