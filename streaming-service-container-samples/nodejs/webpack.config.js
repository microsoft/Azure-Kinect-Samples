const path = require("path");
const HtmlWebPackPlugin = require("html-webpack-plugin");
const CopyWebpackPlugin = require("copy-webpack-plugin");

module.exports = 
{
  entry: 
  [
    __dirname + "/src/depth.html",
    __dirname + "/src/infrared.html",
    __dirname + "/src/multiplex.html",
    __dirname + "/src/cognitive.html",
    __dirname + "/src/color.html",
    __dirname + "/src/examples.css",
    __dirname + "/src/core.css",
    __dirname + "/src/azurekinect.jpg",
    __dirname + "/src/latest.woff2"
  ],
  output: 
  {
    path: path.resolve(__dirname, "dist"),
    filename: "js/[name].js"
  },
  module: 
  {
    rules: 
    [
      {
        test: [/\.js$/],
        exclude: /node_modules/,
        include: [ path.resolve(__dirname, "src/js") ],
        use: ['babel-loader'],
      },
      {
        test: /\.html$/,
        exclude: /node_modules/,
        use: [ 'html-loader'],
      },
      {
        test: [/\.json$/, /\.(jpe?g|gif|png|svg|woff|woff2|ttf|wav|mp3)$/],
        exclude: /node_modules/,
        use: [ 'file-loader'],
      },
      {
        test: /\.css$/,
        exclude: /node_modules/,
        use: ['style-loader', 'css-loader'],
      }
    ],
  },
  devtool: 'source-map',
  plugins: [
    new HtmlWebPackPlugin({
      title: 'Kinect for Azure - Depth',
      template: __dirname + "/src/depth.html",
      filename: "depth.html"
    }),
    new HtmlWebPackPlugin({
      title: 'Kinect for Azure - Infrared',
      template: __dirname + "/src/infrared.html",
      filename: "infrared.html"
    }),
    new HtmlWebPackPlugin({
      title: 'Kinect for Azure - Stream Multiplexing',
      template: __dirname + "/src/multiplex.html",
      filename: "multiplex.html"
    }),
    new HtmlWebPackPlugin({
      title: 'Kinect for Azure - Cognitive Services',
      template: __dirname + "/src/cognitive.html",
      filename: "cognitive.html"
    }),
    new HtmlWebPackPlugin({
      title: 'Kinect for Azure - Color Camera',
      template: __dirname + "/src/color.html",
      filename: "color.html"
    }),    
],
  devServer: {
    port: 3001
  }
};
