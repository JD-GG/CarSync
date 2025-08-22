# Stage 1: Angular Build
FROM node:20 AS build

WORKDIR /usr/src/app
COPY package*.json ./
RUN npm install
COPY . .
RUN npm run build --prod

# Stage 2: Nginx
FROM nginx:alpine
COPY --from=build /usr/src/app/dist/dummy-angular-app /usr/share/nginx/html
COPY nginx/nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
