(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Opposite Columns";
        algo.author = "Heikki Junnila";

        algo.rgbMap = function(width, height, rgb, step)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if ((x % 2) == 0)
                    {
                        if (y == step)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                    else
                    {
                        if (y == (height - step))
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                }
            }

            return map;
        }

        algo.rgbMapStepCount = function(width, height)
        {
            return width;
        }

        return algo;
    }
)()
