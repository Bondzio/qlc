(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Single Random";
        algo.author = "Heikki Junnila";
        algo.initialized = false;
        algo.rgb = 0;

        var util = new Object;
        util.createStepList = function(width, height)
        {
            var list = new Array(height * width);
            var i = 0;
            for (var y = 0; y < height; y++)
            {
                for (var x = 0; x < width; x++)
                {
                    list[i] = [y, x];
                    i++;
                }
            }

            return list;
        }

        util.createStep = function(width, height, sx, sy, rgb)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array(width);
                for (var x = 0; x < width; x++)
                {
                    if (sy == y && sx == x)
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                }
            }

            return map;
        }

        algo.rgbMap = function(width, height, rgb, step)
        {
            // Create a new step list when the color changes
            if (algo.rgb != rgb)
            {
                algo.steps = new Array(width * height);
                var stepList = util.createStepList(width, height);
                for (var i = 0; i < width * height; i++)
                {
                    var index = Math.floor(Math.random() * (stepList.length));
                    var yx = stepList[index];
                    var map = util.createStep(width, height, yx[0], yx[1], rgb);
                    algo.steps[i] = map;
                    stepList.splice(index, 1);
                }

                algo.rgb = rgb;
            }

            return algo.steps[step];
        }

        algo.rgbMapStepCount = function(width, height)
        {
            return width * height;
        }

        return algo;
    }
)()
